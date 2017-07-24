#include "state.h"
#include "debug.h"
#include "win_types.h"
#include "virgl_command.h"


namespace State
{
#define DEFAULT_VGL_CTX 2

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

        framebuffer_id = 0;
        fragshader_id = 0;
        vertshader_id = 0;
        dsa_id = 0;
        vertex_buffer_id = 0;

        restricted = FALSE;
        model_builder = NULL;
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

        VirGL::VirglCommandBuffer cmd(id);
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

        VirGL::VirglCommandBuffer cmd(current_sub_ctx);

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
        if (current_sub_ctx == 0)
            return STATE_ERROR_INVALID_CURRENT_CONTEXT;
        if (!states[current_sub_ctx])
            return STATE_ERROR_INVALID_CURRENT_CONTEXT;

        states[current_sub_ctx]->restricted = TRUE;
        states[current_sub_ctx]->model_builder = new std::vector<model_component_t>();
        return STATUS_SUCCESS;
    }

    INT end(VOID)
    {
        if (current_sub_ctx == 0)
            return STATE_ERROR_INVALID_CURRENT_CONTEXT;
        if (!states[current_sub_ctx])
            return STATE_ERROR_INVALID_CURRENT_CONTEXT;

        states[current_sub_ctx]->restricted = FALSE;
        delete states[current_sub_ctx]->model_builder;

        return STATUS_SUCCESS;
    }
}