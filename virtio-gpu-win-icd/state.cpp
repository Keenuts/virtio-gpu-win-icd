#include "debug.h"
#include "state.h"
#include "tmp_const.h"
#include "virgl.h"
#include "virgl_command.h"
#include "win_types.h"


namespace State
{
#define DEFAULT_VGL_CTX 2
#define DEFAULT_FRAG_HANDLE 4
#define DEFAULT_VERT_HANDLE 5
#define DEFAULT_RASTERIZER_HANDLE 6
#define DEFAULT_BLEND_HANDLE 7

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

    enum ObjectType {
        VERT_SHADER_TYPE = 0,
        FRAG_SHADER_TYPE = 1,
    };

    typedef struct _SHADER_INFO {
        UINT32 type;
        UINT32 binding;
        UINT32 *tokens;
        UINT32 token_count;
        UINT32 offlen;
        UINT32 num_so_output;
    } SHADER_INFO;

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

        framebuffer_id = 0;
        frag_shader_id = 0;
        vert_shader_id = 0;
        rasterizer_id = 0;
        blend_id = 0;
        vertex_buffer_id = 0;

        restricted = FALSE;
        vertex_array = NULL;
        color_array = NULL;
    }

    VOID initializeState(VOID)
    {
        memset(&states, 0, sizeof(states));
        DbgPrint(TRACE_LEVEL_INFO, ("[*] Initializing state tracker.\n"));
    }

    static VOID createVglCtx(VOID)
    {
        states[0] = new OpenGLState();
        current_vgl_ctx = DEFAULT_VGL_CTX;
        current_sub_ctx = 0;

        VirGL::createContext(DEFAULT_VGL_CTX);
    }

    static VOID deleteVglCtx(VOID)
    {
        current_sub_ctx = 0;
        current_vgl_ctx = 0;
        VirGL::deleteContext(DEFAULT_VGL_CTX);
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

        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);
        cmd.createSubContext(i);
        cmd.setCurrentSubContext(i);
        cmd.submitCommandBuffer();
        current_sub_ctx = i;
        *id = i;

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT makeCurrent(UINT32 id)
    {
        if (id >= MAX_STATE_COUNT)
            return STATE_ERROR_INVALID_ID;
        if (!states[id])
            return STATE_ERROR_INVALID_CONTEXT;

        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);
        cmd.setCurrentSubContext(id);
        cmd.submitCommandBuffer();
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

        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);
        cmd.deleteSubContext(id);
        cmd.setCurrentSubContext(0);
        cmd.submitCommandBuffer();

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

        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);

        cmd.clear(states[current_sub_ctx]->clear_color,
                  states[current_sub_ctx]->clear_depth,
                  states[current_sub_ctx]->clear_stencil);
        cmd.submitCommandBuffer();

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
        states[current_sub_ctx]->vertex_array = new std::vector<float>();
        states[current_sub_ctx]->color_array = new std::vector<float>();
        return STATUS_SUCCESS;
    }

    INT end(VOID)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (!states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        states[current_sub_ctx]->restricted = FALSE;
        delete states[current_sub_ctx]->vertex_array;
        delete states[current_sub_ctx]->color_array;
        states[current_sub_ctx]->vertex_array = NULL;
        states[current_sub_ctx]->color_array = NULL;

        return STATUS_SUCCESS;
    }

    static INT loadShader(UINT32 handle, SHADER_INFO shader_info)
    {
        TRACE_IN();

        assert(shader_info.tokens);
        assert(shader_info.token_count);

        BOOL res = 0;
        VirGL::RESOURCE_CREATION info = { 0 };
        info.array_size = 1;
        info.bind = shader_info.binding;
        info.depth = 1;
        info.width = 65536;
        info.height = 1;
        info.handle = handle;

        VirGL::create_resource_3d(current_vgl_ctx, info);
        VirGL::attach_resource(current_vgl_ctx, handle);

        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);

        UINT32 nb_cells = (UINT32)_CMATH_::ceil((double)shader_info.token_count / 4.0);
        DbgPrint(TRACE_LEVEL_WARNING, ("Creating a shader contained in %d cells\n", nb_cells));

        std::vector<UINT32> create_info(4 + nb_cells);
        create_info[0] = shader_info.type;
        create_info[1] = shader_info.token_count;
        create_info[2] = shader_info.offlen;
        create_info[3] = shader_info.num_so_output;
        for (UINT32 i = 0; i < nb_cells; i++)
            create_info[i + 4] = shader_info.tokens[i];

        cmd.createObject(handle, VIRGL_OBJECT_SHADER, create_info);

        cmd.bindShader(handle, shader_info.type);
        res = cmd.submitCommandBuffer();

        if (res != STATUS_SUCCESS)
            DbgPrint(TRACE_LEVEL_ERROR, ("[!] Unable to load the shader id=0x%x\n", handle));
        assert(res == STATUS_SUCCESS);

        TRACE_OUT();
        return STATUS_SUCCESS;
#if 0
        assert(shader_info.tokens);
        assert(shader_info.token_count);

        BOOL res = 0;
        VirGL::RESOURCE_CREATION info;
        memset(&info, 0, sizeof(VirGL::RESOURCE_CREATION));
        info.array_size = 1;
        info.bind = shader_info.binding;
        info.depth = 1;

        //FIXME: proper size managent ?
        info.width = 65536;
        info.height = 1;
        info.handle = handle;

        VirGL::create_resource_3d(current_vgl_ctx, info);
        VirGL::attach_resource(current_vgl_ctx, handle);

        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);

        UINT32 nb_cells = (UINT32)_CMATH_::ceil((double)shader_info.token_count / 4.0);
        DbgPrint(TRACE_LEVEL_WARNING, ("Creating a shader contained in %d cells\n", nb_cells));

        std::vector<UINT32> create_info(4 + nb_cells);
        create_info[0] = shader_info.type;
        create_info[1] = shader_info.token_count;
        create_info[2] = shader_info.offlen;
        create_info[3] = shader_info.num_so_output;
        for (UINT32 i = 0; i < nb_cells; i++)
            create_info[i + 4] = shader_info.tokens[i];

        cmd.createObject(handle, VIRGL_OBJECT_SHADER, create_info);

        cmd.bindShader(handle, shader_info.type);
        res = cmd.submitCommandBuffer();

        if (res != STATUS_SUCCESS)
            DbgPrint(TRACE_LEVEL_ERROR, ("[!] Unable to load the shader id=0x%x\n", handle));
        assert(res == STATUS_SUCCESS);

        TRACE_OUT();
        return STATUS_SUCCESS;
#endif
    }

    INT loadDefaultFragmentShader(VOID)
    {
        TRACE_IN();
        UINT32 handle = DEFAULT_FRAG_HANDLE;
        SHADER_INFO info = { 0 };
        info.binding = 262154;
        info.num_so_output = TmpConst::shader_frag_num_so_output;
        info.token_count = TmpConst::shader_frag_num_tokens;
        info.offlen = TmpConst::shader_frag_offlen;
        info.tokens = TmpConst::shader_frag;
        info.type = FRAG_SHADER_TYPE;

        INT res = loadShader(handle, info);

        if (res != STATUS_SUCCESS)
            DbgPrint(TRACE_LEVEL_ERROR, ("[!] Unable to load default frag %s(0x%x)", errorToStr(res), res));
        assert(res == STATUS_SUCCESS);

        DbgPrint(TRACE_LEVEL_INFO, ("[?] Default fragment shader loaded\n"));
        states[current_sub_ctx]->frag_shader_id = handle;
        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT loadDefaultVertexShader(VOID)
    {
        TRACE_IN();

        UINT32 handle = DEFAULT_VERT_HANDLE;
        SHADER_INFO info = { 0 };
        info.binding = 0x10;
        info.num_so_output = TmpConst::shader_vert_num_so_output;
        info.token_count = TmpConst::shader_vert_num_tokens;
        info.offlen = TmpConst::shader_vert_offlen;
        info.tokens = TmpConst::shader_vert;
        info.type = VERT_SHADER_TYPE;

        INT res = loadShader(handle, info);

        if (res != STATUS_SUCCESS)
            DbgPrint(TRACE_LEVEL_ERROR, ("[!] Unable to load default vert %s(0x%x)", errorToStr(res), res));
        assert(res == STATUS_SUCCESS);

        DbgPrint(TRACE_LEVEL_INFO, ("[?] Default vertex shader loaded\n"));
        states[current_sub_ctx]->vert_shader_id = handle;
        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT loadDefaultRasterizer(VOID)
    {
        TRACE_IN();
        UINT32 handle = DEFAULT_RASTERIZER_HANDLE;

        BOOL res = 0;
        VirGL::RESOURCE_CREATION info;
        memset(&info, 0, sizeof(VirGL::RESOURCE_CREATION));
        info.array_size = 1;
        info.bind = 0x20000;
        info.depth = 1;
        info.width = 8;
        info.height = 1;
        info.handle = handle;
        info.format = 40;

        VirGL::create_resource_3d(current_vgl_ctx, info);
        VirGL::attach_resource(current_vgl_ctx, handle);

        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);

#define SET_BIT_AT(Var, Value, Pos) Var = (Var & ~(1 << Pos)) | (Value << Pos)

        UINT32 bitfield1 = 0;
        UINT32 bitfield2 = 0;
        SET_BIT_AT(bitfield1, 1, 1); //Enable depth clip
        SET_BIT_AT(bitfield1, 1, 7); //point quad rasterization
        SET_BIT_AT(bitfield1, 1, 9); //fill front
        SET_BIT_AT(bitfield1, 1, 15); //offset line
        SET_BIT_AT(bitfield1, 1, 29); // ??
        SET_BIT_AT(bitfield1, 1, 30); // ??

        bitfield2 = 0xffff; //line_stipple_pattern, don't ask me why
#undef SET_BIT_AT

        DbgPrint(TRACE_LEVEL_ERROR, ("Value Bitfield1: 0x%x\n", bitfield1));
        DbgPrint(TRACE_LEVEL_ERROR, ("Value Bitfield2: 0x%x\n", bitfield2));

        std::vector<UINT32> create_info(8);
        create_info[0] = bitfield1;
        create_info[1] = 0x3f800000; //point size = 1.0f
        create_info[2] = 0; //Sprit coord enabled ?
        create_info[3] = bitfield2;
        create_info[3] = 0x3f800000; //line width = 1.0f
        create_info[5] = 0; // offset units
        create_info[6] = 0; // offset scale
        create_info[7] = 0; //offset clamp

        cmd.createObject(handle, VIRGL_OBJECT_RASTERIZER, create_info);
        cmd.bindObject(handle, VIRGL_OBJECT_RASTERIZER);

        res = cmd.submitCommandBuffer();

        if (res != STATUS_SUCCESS)
            DbgPrint(TRACE_LEVEL_ERROR, ("[!] Unable to load default Rasterizer %s(0x%x)", errorToStr(res), res));
        assert(res == STATUS_SUCCESS);

        DbgPrint(TRACE_LEVEL_INFO, ("[?] Default rasterizer loaded\n"));
        states[current_sub_ctx]->blend_id = handle;

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    static INT loadDefaultBlend(VOID)
    {
        TRACE_IN();

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT flush(VOID)
    {
        CHECK_VALID_CTX(stages, current_sub_ctx);
        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        INT res = 0;

        if (states[current_sub_ctx]->frag_shader_id == 0)
            res = loadDefaultFragmentShader();
        assert(res == STATUS_SUCCESS);

        if (states[current_sub_ctx]->vert_shader_id == 0)
            res = loadDefaultVertexShader();
        assert(res == STATUS_SUCCESS);

        if (states[current_sub_ctx]->rasterizer_id == 0)
            res = loadDefaultRasterizer();
        assert(res == STATUS_SUCCESS);

        if (states[current_sub_ctx]->blend_id == 0)
            res = loadDefaultBlend();
        assert(res == STATUS_SUCCESS);

        assert(res == STATUS_SUCCESS);

        return STATUS_SUCCESS;
    }
}