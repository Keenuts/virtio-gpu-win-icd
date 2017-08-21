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
#define DEFAULT_VERTEX_BUFFER_HANDLE 5
#define DEFAULT_RASTERIZER_HANDLE 6
#define DEFAULT_BLEND_HANDLE 7
#define DEFAULT_VERTEX_ELEMENTS_HANDLE 8

namespace State
{
    typedef struct _VBO_ENTRY {
        BOOL valid;

        UINT32 res_handle;
        UINT32 size;
        BOOL enabled;
        BOOL created;
    } VBO_ENTRY, *PVBO_ENTRY;

    typedef struct _VBO_DESCRIPTOR {
        BOOL valid;

        BOOL enabled;
        UINT32 stride;
        UINT32 offset;
        UINT32 res_handle;
    } VBO_DESCRIPTOR, *PVBO_DESCRIPTOR;

    struct OpenGLState
    {
        BOOL restricted;

        FLOAT clear_color[4];
        double clear_depth;
        UINT32 clear_stencil;

        VirGL::RESOURCE_CREATION *frag_shader_info;
        VirGL::RESOURCE_CREATION *vert_shader_info;
        VirGL::RESOURCE_CREATION *rasterizer_info;
        VirGL::RESOURCE_CREATION *blend_info;
        VirGL::RESOURCE_CREATION *vertex_elements_info;

        std::vector<VBO_ENTRY> *vbos;
        std::vector<VBO_DESCRIPTOR> *vbo_descriptors;

        VirGL::VirglCommandBuffer *command_buffer;

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

    INT GenBuffers(UINT32 n, UINT32 *buffers);
    INT BindBuffer(UINT32 target, UINT32 buffer);
    INT BufferData(UINT32 target, UINT32 size, CONST VOID *data, UINT32 usage);
    INT BufferSubData(UINT32 target, UINT32 offset, UINT32 size, CONST VOID *data);
    INT VertexAttribPointer(UINT32 index, UINT32 size, UINT32 type, BOOLEAN normalized, UINT32 stride, CONST VOID *pointer);
    INT EnableVertexAttribArray(UINT32 index);

    INT createDefaultFragmentShader(VOID);
    INT createDefaultVertexShader(VOID);
    INT createDefaultRasterizer(VOID);
    INT createDefaultBlend(VOID);
    INT createDefaultVertexElements(VOID);

    INT loadDefaultFragmentShader(VirGL::VirglCommandBuffer *cmd);
    INT loadDefaultVertexShader(VirGL::VirglCommandBuffer *cmd);
    INT loadDefaultRasterizer(VirGL::VirglCommandBuffer *cmd);
    INT loadDefaultBlend(VirGL::VirglCommandBuffer *cmd);
    // INT loadDefaultVertexElement(VirGL::VirglCommandBuffer& cmd);

    INT setDefaultPolygonStipple(VirGL::VirglCommandBuffer *cmd);
    INT setDefaultScissorState(VirGL::VirglCommandBuffer *cmd);
    INT setDefaultViewportState(VirGL::VirglCommandBuffer *cmd);
    INT setDefaultFramebuffer(VirGL::VirglCommandBuffer *cmd);
    // INT setDefaultConstantBuffers(VirGL::VirglCommandBuffer& cmd);

    CONST CHAR* errorToStr(INT error);
}