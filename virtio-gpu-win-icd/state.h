#pragma once

#include <vector>

#include "virgl_command.h"
#include "uniform_buffer.h"
#include "win_types.h"

#define MAX_STATE_COUNT 3

#define DEFAULT_VGL_CTX 2
#define DEFAULT_FRAMEBUFFER_HANDLE 0
#define DEFAULT_FRAG_HANDLE 4
#define DEFAULT_VERT_HANDLE 5
#define DEFAULT_RASTERIZER_HANDLE 6
#define DEFAULT_BLEND_HANDLE 7
#define DEFAULT_VERTEX_ELEMENTS_HANDLE 8

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
        UniformBuffer<float> *color_buffer;

        BOOL initialized;
        VirGL::RESOURCE_CREATION *frag_shader_info;
        VirGL::RESOURCE_CREATION *vert_shader_info;
        VirGL::RESOURCE_CREATION *rasterizer_info;
        VirGL::RESOURCE_CREATION *blend_info;
        VirGL::RESOURCE_CREATION *vertex_elements_info;

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

    INT createDefaultFragmentShader(VOID);
    INT createDefaultVertexShader(VOID);
    INT createDefaultRasterizer(VOID);
    INT createDefaultBlend(VOID);
    INT createDefaultVertexElements(VOID);

    INT loadDefaultFragmentShader(VirGL::VirglCommandBuffer& cmd);
    INT loadDefaultVertexShader(VirGL::VirglCommandBuffer& cmd);
    INT loadDefaultRasterizer(VirGL::VirglCommandBuffer& cmd);
    INT loadDefaultBlend(VirGL::VirglCommandBuffer& cmd);
    INT loadDefaultPolygonStipple(VirGL::VirglCommandBuffer& cmd);

    CONST CHAR* errorToStr(INT error);
}