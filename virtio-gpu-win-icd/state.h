#pragma once

#include <vector>

#include "uniform_buffer.h"
#include "win_types.h"

#define MAX_STATE_COUNT 3


namespace State
{
    struct OpenGLState
    {
        BOOL restricted;

        FLOAT clear_color[4];
        double clear_depth;
        UINT32 clear_stencil;

        UINT32 framebuffer_id;
        UINT32 frag_shader_id;
        UINT32 vert_shader_id;
        UINT32 rasterizer_id;
        UINT32 blend_id;

        UniformBuffer<float> *vertex_buffer;

        OpenGLState();
        ~OpenGLState();
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

    INT push_color(float v);
    INT push_vertex(float v);

    INT end(VOID);

    INT flush(VOID);

    CONST CHAR* errorToStr(INT error);
}