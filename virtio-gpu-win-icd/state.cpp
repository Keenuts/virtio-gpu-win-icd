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

    enum StateError {
        STATE_ERROR_INVALID_ID = 1,
        STATE_ERROR_CONTEXT_EXISTS,
        STATE_ERROR_CANNOT_ALLOC_CONTEXT,
        STATE_ERROR_INVALID_CONTEXT,
        STATE_ERROR_INVALID_CURRENT_CONTEXT,
        STATE_ERROR_NOT_ALLOWED,
    };

    CONST CHAR* errorToStr(INT error)
    {
#define ERRTOSTR(Error) case Error: return #Error
        switch (error)
        {
        ERRTOSTR(STATUS_SUCCESS);
        ERRTOSTR(STATE_ERROR_INVALID_ID);
        ERRTOSTR(STATE_ERROR_CONTEXT_EXISTS);
        ERRTOSTR(STATE_ERROR_CANNOT_ALLOC_CONTEXT);
        ERRTOSTR(STATE_ERROR_INVALID_CONTEXT);
        ERRTOSTR(STATE_ERROR_INVALID_CURRENT_CONTEXT);
        ERRTOSTR(STATE_ERROR_NOT_ALLOWED);
        default:
            return "UNKNOWN";
        }
#undef ERRTOSTR
    }

    OpenGLState::OpenGLState()
    {
        clear_depth = 1.0;
        clear_stencil = 0;
        memset(clear_color, 0, sizeof(clear_color));

        frag_shader_info = NULL;
        vert_shader_info = NULL;
        rasterizer_info = NULL;
        blend_info = NULL;
        vertex_elements_info = NULL;

        restricted = FALSE;
        command_buffer = new VirGL::VirglCommandBuffer(current_vgl_ctx);

        vbos = new std::vector<VBO_ENTRY>();
        vbo_descriptors = new std::vector<VBO_DESCRIPTOR>();
    }

    OpenGLState::~OpenGLState()
    {
    }

    VOID initializeState(VOID)
    {
        memset(&states, 0, sizeof(states));
        DbgPrint(TRACE_LEVEL_INFO, ("[*] Initializing state tracker.\n"));
    }

    static VOID createVglCtx(VOID)
    {
        UINT32 res = STATUS_SUCCESS;

        states[0] = new OpenGLState();
        current_vgl_ctx = DEFAULT_VGL_CTX;
        current_sub_ctx = 0;

        VirGL::createContext(DEFAULT_VGL_CTX);
        VirGL::attachResource(current_vgl_ctx, DEFAULT_FRAMEBUFFER_HANDLE);

        res = createDefaultFragmentShader();
        assert(res == STATUS_SUCCESS);
        res = createDefaultVertexShader();
        assert(res == STATUS_SUCCESS);
        res = createDefaultRasterizer();
        assert(res == STATUS_SUCCESS);
        res = createDefaultBlend();
        assert(res == STATUS_SUCCESS);
        res = createDefaultVertexElements();
        assert(res == STATUS_SUCCESS);
    }

    static VOID deleteVglCtx(VOID)
    {
        current_sub_ctx = 0;
        current_vgl_ctx = 0;
        VirGL::deleteContext(DEFAULT_VGL_CTX);
    }

    static VOID setupContextDefaults(VOID)
    {
        TRACE_IN();

        INT res = 0;
        VirGL::VirglCommandBuffer *cmd = states[current_sub_ctx]->command_buffer;

        cmd->setCurrentSubContext(0);

        std::vector<UINT32> args(4);
        args[0] = 0x5;
        args[1] = 0x1;
        args[2] = 0x0;
        args[3] = 0x0;
        cmd->createObject(2, VIRGL_OBJECT_SURFACE, args);

        args.resize(4);
        args[0] = 0x0;
        args[1] = 0x0;
        args[2] = 0x0;
        args[3] = 0x0;
        cmd->createObject(3, VIRGL_OBJECT_DSA, args);
        cmd->bindObject(3, VIRGL_OBJECT_DSA);

        res = loadDefaultFragmentShader(cmd);
        res = loadDefaultVertexShader(cmd);
        res = loadDefaultRasterizer(cmd);
        res = loadDefaultBlend(cmd);

        TRACE_OUT();
    }

    INT createContext(UINT32 *id)
    {
        TRACE_IN();
        assert(id);

        UINT32 i;
        for (i = 1; i < MAX_STATE_COUNT; i++)
            if (!states[i])
                break;

        if (i >= MAX_STATE_COUNT)
            return STATE_ERROR_CANNOT_ALLOC_CONTEXT;

        if (current_vgl_ctx == 0)
            createVglCtx();

        states[i] = new OpenGLState();

        VirGL::VirglCommandBuffer *cmd = states[i]->command_buffer;
        cmd->createSubContext(i);
        cmd->setCurrentSubContext(i);
        cmd->submitCommandBuffer();

        current_sub_ctx = i;
        *id = i;

        setupContextDefaults();

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT makeCurrent(UINT32 id)
    {
        if (id >= MAX_STATE_COUNT)
            return STATE_ERROR_INVALID_ID;
        if (!states[id])
            return STATE_ERROR_INVALID_CONTEXT;

        VirGL::VirglCommandBuffer *cmd = states[current_sub_ctx]->command_buffer;
        cmd->setCurrentSubContext(id);
        current_sub_ctx = id;

        return STATUS_SUCCESS;
    }

    INT deleteContext(UINT32 id)
    {
        if (id == 0 || id >= MAX_STATE_COUNT)
            return STATE_ERROR_INVALID_ID;
        if (!states[id])
            return STATE_ERROR_INVALID_CONTEXT;

        delete states[id];
        states[id] = NULL;
        current_sub_ctx = 0;

        VirGL::VirglCommandBuffer *cmd = states[current_sub_ctx]->command_buffer;

        cmd->emptyCommandBuffer();

        cmd->deleteSubContext(id);
        cmd->setCurrentSubContext(0);
        cmd->submitCommandBuffer();

        for (UINT32 i = 1; i < MAX_STATE_COUNT; i++)
            if (states[i])
                return STATUS_SUCCESS;

        deleteVglCtx();
        return STATUS_SUCCESS;
    }

#define CHECK_VALID_CTX(States, Current_sub_ctx)        \
    do {                                                \
        if (current_sub_ctx == 0)                       \
            return STATE_ERROR_INVALID_CURRENT_CONTEXT; \
        if (!states[current_sub_ctx])                   \
            return STATE_ERROR_INVALID_CURRENT_CONTEXT; \
    } while (0)

    INT clear(VOID)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        VirGL::VirglCommandBuffer *cmd = states[current_sub_ctx]->command_buffer;

        cmd->clear(states[current_sub_ctx]->clear_color,
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

    INT begin(VOID)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        states[current_sub_ctx]->restricted = TRUE;

        return STATUS_SUCCESS;
    }

    INT push_vertex(float v)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (!states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        UNREFERENCED_PARAMETER(v);

        return STATUS_SUCCESS;
    }

    INT push_color(float c)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (!states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        UNREFERENCED_PARAMETER(c);

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

            entry.res_handle = DEFAULT_VERTEX_BUFFER_HANDLE;
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

    static INT retreiveCurrentVBO(UINT32 *handle)
    {

        std::vector<VBO_ENTRY> *vbos = states[current_sub_ctx]->vbos;
        UINT32 current = (UINT32)-1;
        UINT32 i;

        assert(handle);

        for (i = 0; i < vbos->size(); i++) {
            if ((*vbos)[i].enabled) {
                current = i;
                break;
            }
        }

        if (current >= vbos->size() || !(*vbos)[current].valid)
            return STATUS_INVALID_HANDLE;
        *handle = current;
        return STATUS_SUCCESS;
    }

    INT BufferData(UINT32 target, UINT32 size, CONST VOID *data, UINT32 usage)
    {
        std::vector<VBO_ENTRY> *vbos = states[current_sub_ctx]->vbos;
        UINT current;
        UINT32 res = retreiveCurrentVBO(&current);
        if (res != STATUS_SUCCESS)
            return res;

        (*vbos)[current].size = size;

        UNREFERENCED_PARAMETER(data);
        UNREFERENCED_PARAMETER(usage);
        UNREFERENCED_PARAMETER(target);

        return STATUS_SUCCESS;
    }

    INT BufferSubData(UINT32 target, UINT32 offset, UINT32 size, CONST VOID *data)
    {
        std::vector<VBO_ENTRY> *vbos = states[current_sub_ctx]->vbos;
        VirGL::VirglCommandBuffer *cmd = states[current_sub_ctx]->command_buffer;
        VirGL::INLINE_WRITE info = { 0 };
        UINT current;
        UINT32 res = retreiveCurrentVBO(&current);
        if (res != STATUS_SUCCESS)
            return res;

        info.handle = (*vbos)[current].res_handle;
        info.data = (VOID*)data;
        info.data_len = size;
        info.x = offset;
        info.width = size;
        info.height = 1;
        info.depth = 1;
        info.usage = size;

        cmd->setCurrentSubContext(0);
        cmd->inlineWrite(info);
        cmd->submitCommandBuffer();

        UNREFERENCED_PARAMETER(data);
        UNREFERENCED_PARAMETER(target);

        return STATUS_SUCCESS;
    }

    static UINT32 sizeFromType(UINT32 type)
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

    INT VertexAttribPointer(UINT32 index, UINT32 size, UINT32 type, BOOLEAN normalized, UINT32 stride, CONST VOID *pointer)
    {
        std::vector<VBO_DESCRIPTOR> *descriptors = states[current_sub_ctx]->vbo_descriptors;
        VBO_DESCRIPTOR descriptor = { 0 };
        UINT current;
        UINT32 res = retreiveCurrentVBO(&current);
        if (res != STATUS_SUCCESS)
            return res;

        descriptor.valid = TRUE;
        descriptor.offset = (UINT32)(UINT64)pointer;
        descriptor.res_handle = states[current_sub_ctx]->vbos->data()[current].res_handle;
        descriptor.stride = size * sizeFromType(type);

        if ((UINT32)descriptors->size() < index + 1)
            descriptors->resize(index + 1);
        descriptors->data()[index] = descriptor;

        UNREFERENCED_PARAMETER(stride);
        UNREFERENCED_PARAMETER(normalized);
        return STATUS_SUCCESS;
    }

    INT EnableVertexAttribArray(UINT32 index)
    {
        std::vector<VBO_DESCRIPTOR> *descriptors = states[current_sub_ctx]->vbo_descriptors;
        if ((UINT32)descriptors->size() < index + 1 || !(*descriptors)[index].valid)
            return STATUS_INVALID_HANDLE;
        (*descriptors)[index].enabled = TRUE;
        return STATUS_SUCCESS;
    }
}