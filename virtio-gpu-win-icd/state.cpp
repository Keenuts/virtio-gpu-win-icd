#include "debug.h"
#include "driver_api.h"
#include "GLtypes.h"
#include "state.h"
#include "tmp_const.h"
#include "uniform_buffer.h"
#include "virgl.h"
#include "virgl_command.h"
#include "win_types.h"

namespace State
{
    UINT32 current_sub_ctx;
    UINT32 current_vgl_ctx;
    OpenGLState *states[MAX_STATE_COUNT];

    OpenGLState::OpenGLState()
    {
        restricted = FALSE;

        resource_index = 3;
        object_index = 1;

        memset(clear_color, 0, sizeof(clear_color));
        clear_depth = 1.0;
        clear_stencil = 0;

        vertex_buffer_info = { 0 };
        surface_info = { 0 };
        depth_buffer_info = { 0 };

        unknown_res_info_0 = { 0 };
        unknown_res_info_2 = { 0 };

        frag_shader_info = { 0 };
        vert_shader_info = { 0 };

        framebuffer_handle = 0;
        frag_shader_handle = 0;
        vert_shader_handle = 0;
        DSA_handle = 0;
        rasterizer_handle = 0;
        blend_handle = 0;
        vertex_elements_handle = 0;

        vbos = new std::vector<VBO_ENTRY>();
        vbo_descriptors = new std::vector<VBO_DESCRIPTOR>();

        command_buffer = new VirGL::VirglCommandBuffer(current_vgl_ctx);
    }

    OpenGLState::~OpenGLState()
    {
    }

    VOID initializeState(VOID)
    {
        memset(&states, 0, sizeof(states));
        DbgPrint(TRACE_LEVEL_INFO, ("[*] Initializing state tracker.\n"));
    }

#define CHECK_VALID_CTX(States, Current_sub_ctx)        \
    do {                                                \
        if (!states[current_sub_ctx])                   \
            return STATE_ERROR_INVALID_CURRENT_CONTEXT; \
    } while (0)

    INT clear(UINT32 mask)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        VirGL::VirglCommandBuffer *cmd = states[current_sub_ctx]->command_buffer;

        cmd->clear(mask,
                  states[current_sub_ctx]->clear_color,
                  states[current_sub_ctx]->clear_depth,
                  states[current_sub_ctx]->clear_stencil);

        return STATUS_SUCCESS;
    }

    INT clearColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        states[current_sub_ctx]->clear_color[0] = r;
        states[current_sub_ctx]->clear_color[1] = g;
        states[current_sub_ctx]->clear_color[2] = b;
        states[current_sub_ctx]->clear_color[3] = a;

        return STATUS_SUCCESS;
    }

    INT clearDepth(double d)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;
        states[current_sub_ctx]->clear_depth = d;
        return STATUS_SUCCESS;
    }

    INT clearStencil(UINT32 s)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;
        states[current_sub_ctx]->clear_stencil = s;
        return STATUS_SUCCESS;
    }

    INT viewport(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        UNREFERENCED_PARAMETER(x);
        UNREFERENCED_PARAMETER(y);
        UNREFERENCED_PARAMETER(width);
        UNREFERENCED_PARAMETER(height);

        return STATUS_SUCCESS;
    }

    INT begin(UINT32 mode)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        states[current_sub_ctx]->restricted = TRUE;

        UNREFERENCED_PARAMETER(mode);
        return STATUS_SUCCESS;
    }

    INT end(VOID)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (!states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        states[current_sub_ctx]->restricted = FALSE;
        return STATUS_SUCCESS;
    }

    INT enable(UINT32 cap)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        UNREFERENCED_PARAMETER(cap);

        return STATUS_SUCCESS;
    }

    INT flush(VOID)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        INT res = 0;
        VirGL::VirglCommandBuffer *cmd = states[current_sub_ctx]->command_buffer;

        res = cmd->submitCommandBuffer();
        assert(res == STATUS_SUCCESS);
        return STATUS_SUCCESS;
    }

    INT GenBuffers(UINT32 n, UINT32 *buffers)
    {
        assert(buffers);

        std::vector<VBO_ENTRY> *vbos = states[current_sub_ctx]->vbos;

        for (UINT32 i = 0; i < n; i++) {
            UINT32 handle = (UINT32)vbos->size();
            vbos->resize(handle + 1);

            VBO_ENTRY entry = { 0 };
            entry.valid = TRUE;

            entry.res_handle = states[current_sub_ctx]->vertex_buffer_info.handle;
            entry.size = 0;
            entry.enabled = FALSE;
            entry.created = FALSE;

            vbos->data()[handle] = entry;
            buffers[i] = handle;
        }

        return STATUS_SUCCESS;
    }

    INT BindBuffer(UINT32 target, UINT32 buffer)
    {
        std::vector<VBO_ENTRY> *vbos = states[current_sub_ctx]->vbos;
        if (buffer > vbos->size() || !(*vbos)[buffer].valid)
            return STATUS_INVALID_HANDLE;

        for (UINT32 i = 0; i < vbos->size(); i++)
            (*vbos)[i].enabled = FALSE;
        (*vbos)[buffer].enabled = TRUE;

        UNREFERENCED_PARAMETER(target);
        return STATUS_SUCCESS;
    }

    static VBO_ENTRY* retreiveCurrentVBO(VOID)
    {
        std::vector<VBO_ENTRY> *vbos = states[current_sub_ctx]->vbos;
        UINT32 i;

        for (i = 0; i < vbos->size(); i++)
            if ((*vbos)[i].enabled)
                break;

        if (i >= vbos->size() || !(*vbos)[i].valid)
            return NULL;
        return &vbos->data()[i];
    }

    INT BufferData(UINT32 target, UINT32 size, CONST VOID *data, UINT32 usage)
    {
        OpenGLState *st = states[current_sub_ctx];
        VBO_ENTRY *vbo = retreiveCurrentVBO();
        if (vbo == NULL)
            return STATE_ERROR_INVALID_VBO;

        vbo->size = size;
        if (!vbo->created) {

            st->vertex_buffer_info = create3DResource(0, 0x40, 0x10, size);
            VirGL::attachResource(current_vgl_ctx, st->vertex_buffer_info.handle);
            vbo->created = TRUE;
            vbo->res_handle = st->vertex_buffer_info.handle;
        }

        UNREFERENCED_PARAMETER(data);
        UNREFERENCED_PARAMETER(usage);
        UNREFERENCED_PARAMETER(target);

        return STATUS_SUCCESS;
    }

    INT BufferSubData(UINT32 target, UINT32 offset, UINT32 size, CONST VOID *data)
    {
        OpenGLState *st = states[current_sub_ctx];
        VBO_ENTRY *vbo = retreiveCurrentVBO();
        VirGL::VirglCommandBuffer *cmd = st->command_buffer;
        VirGL::INLINE_WRITE info = { 0 };

        if (vbo == NULL)
            return STATE_ERROR_INVALID_VBO;

        info.handle = vbo->res_handle;
        info.data = (VOID*)(data);
        info.data_len = size;
        info.x = offset;
        info.width = size;
        info.height = 1;
        info.depth = 1;
        info.usage = size;

        cmd->setCurrentSubContext(current_sub_ctx);
        cmd->inlineWrite(info);
        cmd->submitCommandBuffer();

        UNREFERENCED_PARAMETER(target);
        return STATUS_SUCCESS;
    }

    static UINT32 strideFromType(UINT32 type)
    {
        if (type == GL_FLOAT)
            return sizeof(FLOAT);
        else if (type == GL_INT)
            return sizeof(INT);
        else if (type == GL_UNSIGNED_INT)
            return sizeof(INT);
        else if (type == GL_BYTE)
            return sizeof(BYTE);
        else
            assert(0 && "Unknown type");
        return 1;
    }

    INT VertexAttribPointer(UINT32 index, UINT32 size, UINT32 type, BOOLEAN normalized,
                            UINT32 stride, CONST VOID *pointer)
    {
        OpenGLState *st = states[current_sub_ctx];
        std::vector<VBO_DESCRIPTOR> *dc = st->vbo_descriptors;
        VBO_ENTRY *vbo = retreiveCurrentVBO();
        VBO_DESCRIPTOR descriptor = { 0 };
        UINT32 i;

        if (vbo == NULL)
            return STATE_ERROR_INVALID_VBO;

        descriptor.valid = TRUE;
        descriptor.offset = static_cast<UINT32>(reinterpret_cast<UINT64>(pointer));
        descriptor.res_handle = vbo->res_handle;
        descriptor.stride = size * strideFromType(type);

        if ((UINT32)dc->size() < index + 1) {
            i = (UINT32)dc->size();
            dc->resize(index + 1);

            for (; i < dc->size(); i++)
                (*dc)[i].valid = FALSE;
        }

        dc->data()[index] = descriptor;

        UNREFERENCED_PARAMETER(stride);
        UNREFERENCED_PARAMETER(normalized);
        return STATUS_SUCCESS;
    }

    INT EnableVertexAttribArray(UINT32 index)
    {
        std::vector<VBO_DESCRIPTOR> *dc = states[current_sub_ctx]->vbo_descriptors;

        if ((UINT32)dc->size() < index + 1 || !(*dc)[index].valid)
            return STATE_ERROR_INVALID_INDEX;
        (*dc)[index].enabled = TRUE;

        return STATUS_SUCCESS;
    }

    static INT CreateVertexElements()
    {
        OpenGLState *st = states[current_sub_ctx];
        std::vector<VBO_DESCRIPTOR> *dc = st->vbo_descriptors;
        VirGL::VirglCommandBuffer *cmd = st->command_buffer;
        UINT32 i;

        std::vector<UINT32> args(dc->size() * 4);

        for (i = 0; i < dc->size(); i++) {
                args[i * 4 + 0] = 0;
                args[i * 4 + 1] = 0;
                args[i * 4 + 2] = i;
                args[i * 4 + 3] = 0x1e;
        }

        st->vertex_elements_handle = createObject(VIRGL_OBJECT_VERTEX_ELEMENTS, args);
        cmd->bindObject(st->vertex_elements_handle, VIRGL_OBJECT_VERTEX_ELEMENTS);

        return STATUS_SUCCESS;
    }

    INT DrawArrays(UINT32 mode, UINT32 first, UINT32 count)
    {
        INT res;
        UINT32 i;
        OpenGLState *st = states[current_sub_ctx];
        VirGL::VirglCommandBuffer *cmd = st->command_buffer;
        std::vector<VBO_DESCRIPTOR> *dc = st->vbo_descriptors;
        std::vector<VirGL::VBO_SETTINGS> params(dc->size());
        VirGL::DRAW_VBO_SETTINGS settings = { 0 };

        if (st->vertex_elements_handle == 0) {
            res = CreateVertexElements();
            if (res != STATUS_SUCCESS)
                return res;
        }

        for (i = 0; i < dc->size(); i++) {
            if (!(*dc)[i].valid) {
                DbgPrint(TRACE_LEVEL_ERROR, ("[!] Invalid descriptor found !\n"));
                return STATE_ERROR_INVALID_VBO;
            }
        }

        for (i = 0; i < dc->size(); i++) {
            VirGL::VBO_SETTINGS s = { 0 };
            s.handle = (*dc)[i].res_handle;
            s.offset = (*dc)[i].offset;
            s.stride = (*dc)[i].stride;
            params[i] = s;
        }

        cmd->setVBO(params);

        settings.start = first;
        settings.count = count;
        settings.mode = mode;
        settings.indexed = 0;
        settings.instance_count = 1;
        settings.min_index = first;
        settings.max_index = first + count - 1;

        cmd->drawVBO(settings);
        cmd->submitCommandBuffer();

        return STATUS_SUCCESS;
    }
}
