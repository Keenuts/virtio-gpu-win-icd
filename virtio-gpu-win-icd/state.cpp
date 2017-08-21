#include "debug.h"
#include "driver_api.h"
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
}