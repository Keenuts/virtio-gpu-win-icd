#pragma once

#include <vector>
#include "win_types.h"

#define MAX_STATE_COUNT 3


namespace State
{
    struct model_component_t {
        float r, g, b;
        float x, y, z;
    };

    struct OpenGLState
    {
        BOOL restricted;

        FLOAT clear_color[4];
        double clear_depth;
        UINT32 clear_stencil;

        UINT32 framebuffer_id;
        UINT32 fragshader_id;
        UINT32 vertshader_id;
        UINT32 dsa_id;

        UINT32 vertex_buffer_id;

        std::vector<model_component_t> *model_builder;

        OpenGLState();
    };

    VOID initializeState(VOID);
    INT createContext(UINT32 *id);
    INT makeCurrent(UINT32 id);
    INT deleteContext(UINT32 id);

    INT clear(VOID);
    INT clearColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a);
    INT clearDepth(double depth);
    INT clearStencil(UINT32 s);

    INT begin(VOID);
    INT end(VOID);

    CONST CHAR* errorToStr(INT error);
}