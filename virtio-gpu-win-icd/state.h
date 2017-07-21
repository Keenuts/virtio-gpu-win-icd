#pragma once

#include <vector>
#include "win_types.h"

#define MAX_STATE_COUNT 2


namespace State
{
    struct model_component_t {
        float r, g, b;
        float x, y, z;
    };

    struct OpenGLState
    {
        UINT32 sub_ctx;
        UINT32 vgl_ctx;
        BOOL restricted;


        UINT32 framebuffer_id;
        UINT32 fragshader_id;
        UINT32 vertshader_id;
        UINT32 dsa_id;

        UINT32 vertex_buffer_id;

        std::vector<model_component_t> *model_builder;
    };

    VOID initializeState(VOID);
    INT createContext(UINT32 id);
    INT makeCurrent(UINT32 id);
    INT deleteContext(UINT32 id);
    INT clear(VOID);
    INT begin(VOID);
    INT end(VOID);
}