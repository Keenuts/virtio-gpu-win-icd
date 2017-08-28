#pragma once

#include <vector>

#include "virgl_command.h"
#include "uniform_buffer.h"
#include "win_types.h"

#define MAX_STATE_COUNT 3

#define DEFAULT_VGL_CTX 2
#define WINDOWS_FRAMEBUFFER_HANDLE 1

namespace State
{
    enum StateError {
        STATE_ERROR_INVALID_ID = 1,
        STATE_ERROR_CONTEXT_EXISTS,
        STATE_ERROR_CANNOT_ALLOC_CONTEXT,
        STATE_ERROR_INVALID_CONTEXT,
        STATE_ERROR_INVALID_CURRENT_CONTEXT,
        STATE_ERROR_NOT_ALLOWED,
        STATE_ERROR_INVALID_VBO,
        STATE_ERROR_INVALID_INDEX,
    };

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
        UINT32 resource_index;
        UINT32 object_index;

        FLOAT clear_color[4];
        double clear_depth;
        UINT32 clear_stencil;

        VirGL::RESOURCE_CREATION vertex_buffer_info;
        VirGL::RESOURCE_CREATION surface_info;
        VirGL::RESOURCE_CREATION depth_buffer_info;

        VirGL::RESOURCE_CREATION unknown_res_info_0;
        VirGL::RESOURCE_CREATION unknown_res_info_2;

        VirGL::RESOURCE_CREATION frag_shader_info;
        VirGL::RESOURCE_CREATION vert_shader_info;

        UINT32 framebuffer_handle;
        UINT32 depth_buffer_handle;
        UINT32 frag_shader_handle;
        UINT32 vert_shader_handle;
        UINT32 DSA_handle;
        UINT32 rasterizer_handle;
        UINT32 blend_handle;
        UINT32 vertex_elements_handle;

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

    INT clear(UINT32 mask);
    INT clearColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a);
    INT clearDepth(double depth);
    INT clearStencil(UINT32 s);

    INT viewport(UINT32 x, UINT32 y, UINT32 width, UINT32 height);

    INT begin(UINT32 mode);
    INT end(VOID);

    INT enable(UINT32 cap);
    INT flush(VOID);


    INT GenBuffers(UINT32 n, UINT32 *buffers);
    INT BindBuffer(UINT32 target, UINT32 buffer);
    INT BufferData(UINT32 target, UINT32 size, CONST VOID *data, UINT32 usage);
    INT BufferSubData(UINT32 target, UINT32 offset, UINT32 size, CONST VOID *data);
    INT VertexAttribPointer(UINT32 index, UINT32 size, UINT32 type, BOOLEAN normalized,
                            UINT32 stride, CONST VOID *pointer);
    INT EnableVertexAttribArray(UINT32 index);
    INT DrawArrays(UINT32 mode, UINT32 first, UINT32 count);

    /* Helpers */
    CONST CHAR* errorToStr(INT error);

    VirGL::RESOURCE_CREATION create3DResource
        (
            UINT32 target, UINT32 format, UINT32 bind, UINT32 width,
            UINT32 height = 1, UINT32 depth = 1, UINT32 array_size = 1
        );
    UINT32 createObject(UINT32 type, std::vector<UINT32>& args);
    UINT32 createSurface(UINT32 res_handle, UINT32 format);
    UINT32 createDSA(UINT32 a, UINT32 b, UINT32 c, UINT32 d);

    /* Default setup functions */

    UINT32 createDefaultFragmentShader(VOID);
    UINT32 createDefaultVertexShader(VOID);
    UINT32 createDefaultRasterizer(VOID);
    UINT32 createDefaultBlend(VOID);

    INT setDefaultPolygonStipple(VirGL::VirglCommandBuffer *cmd);
    INT setDefaultScissorState(VirGL::VirglCommandBuffer *cmd);
    INT setDefaultViewportState(VirGL::VirglCommandBuffer *cmd);
    INT setDefaultFramebuffer(VirGL::VirglCommandBuffer *cmd,
                              UINT32 framebufferHandle, UINT32 zbufferHandle);
    INT setDefaultConstantBuffers(VirGL::VirglCommandBuffer *cmd);

}
