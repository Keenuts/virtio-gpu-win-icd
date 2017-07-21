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
        STATE_ERROR_INVALID_ID,
        STATE_ERROR_CONTEXT_EXISTS,
        STATE_ERROR_INVALID_CONTEXT,
        STATE_ERROR_INVALID_CURRENT_CONTEXT,
        STATE_ERROR_NOT_ALLOWED,
    };

    VOID initializeState(VOID)
    {
        memset(&states, 0, sizeof(states));
        DbgPrint(TRACE_LEVEL_INFO, ("[*] Initializing state tracker.\n"));
    }

    INT createContext(UINT32 id)
    {
        TRACE_IN();

        if (id >= MAX_STATE_COUNT)
            return STATE_ERROR_INVALID_ID;
        if (states[id])
            return STATE_ERROR_CONTEXT_EXISTS;

        states[id] = new OpenGLState();
        memset(states[id], 0, sizeof(OpenGLState));

        if (!current_vgl_ctx) {
            current_vgl_ctx = DEFAULT_VGL_CTX;
            VirGL::createContext(DEFAULT_VGL_CTX);
        }

        current_sub_ctx = id;
        VirGL::VirglCommandBuffer cmd(id);
        cmd.createSubContext(id);
        cmd.submitCommandBuffer();

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT makeCurrent(UINT32 id)
    {
        if (id >= MAX_STATE_COUNT)
            return STATE_ERROR_INVALID_ID;
        if (!states[id])
            return STATE_ERROR_INVALID_CONTEXT;

        current_sub_ctx = id;
        VirGL::VirglCommandBuffer cmd(id);
        cmd.setCurrentSubContext(id);
        cmd.submitCommandBuffer();

        return STATUS_SUCCESS;
    }

    INT deleteContext(UINT32 id)
    {
        if (id >= MAX_STATE_COUNT)
            return STATE_ERROR_INVALID_ID;
        if (!states[id])
            return STATE_ERROR_INVALID_CONTEXT;

        delete states[id];
        states[id] = NULL;
        current_sub_ctx = 0;

        VirGL::VirglCommandBuffer cmd(id);
        cmd.deleteSubContext(id);
        cmd.submitCommandBuffer();

        for (UINT32 i = 0; i < MAX_STATE_COUNT; i++)
            if (states[i])
                return STATUS_SUCCESS;

        current_vgl_ctx = 0;
        VirGL::deleteContext(DEFAULT_VGL_CTX);
        return STATUS_SUCCESS;
    }

    INT clear(VOID)
    {
        if (current_sub_ctx == 0)
            return STATE_ERROR_INVALID_CURRENT_CONTEXT;
        if (!states[current_sub_ctx])
            return STATE_ERROR_INVALID_CURRENT_CONTEXT;

        if (states[current_sub_ctx]->restricted)
            return STATE_ERROR_NOT_ALLOWED;

        VirGL::VirglCommandBuffer cmd(current_sub_ctx);
        float color[4] = { 0.0f, 0.0f, 0.5f, 1.0f };

        cmd.setCurrentSubContext(current_sub_ctx);
        cmd.clear(color, 0, 0);
        cmd.submitCommandBuffer();

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