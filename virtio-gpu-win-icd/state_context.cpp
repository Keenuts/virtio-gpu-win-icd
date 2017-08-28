#include "debug.h"
#include "driver_api.h"
#include "GLtypes.h"
#include "state.h"
#include "tmp_const.h"
#include "uniform_buffer.h"
#include "virgl.h"
#include "virgl_command.h"
#include "win_types.h"

namespace State
{
    extern UINT32 current_sub_ctx;
    extern UINT32 current_vgl_ctx;
    extern OpenGLState *states[MAX_STATE_COUNT];

    static VOID createVglCtx(VOID)
    {
        current_vgl_ctx = DEFAULT_VGL_CTX;
        current_sub_ctx = 0;

        VirGL::createContext(current_vgl_ctx);
        VirGL::attachResource(current_vgl_ctx, WINDOWS_FRAMEBUFFER_HANDLE);
    }

    static VOID deleteVglCtx(VOID)
    {
        current_sub_ctx = 0;
        current_vgl_ctx = 0;

        VirGL::detachResource(current_vgl_ctx, WINDOWS_FRAMEBUFFER_HANDLE);
        VirGL::deleteContext(current_vgl_ctx);
    }

    static VOID setupContextDefaults(VOID)
    {
        TRACE_IN();

        OpenGLState *st = states[current_sub_ctx];
        VirGL::VirglCommandBuffer *cmd = st->command_buffer;

        st->frag_shader_info = create3DResource(0, 0x40, 0x10, 0x10000, 1);
        VirGL::attachResource(current_vgl_ctx, st->frag_shader_info.handle);

        st->surface_info = { 0 };

#define USE_WINDOWS_BUFFER 1
#if USE_WINDOWS_BUFFER
        st->surface_info.handle = WINDOWS_FRAMEBUFFER_HANDLE;
#else
        st->surface_info = create3DResource(
            PIPE_TEXTURE_2D,
            VIRGL_FORMAT_B8G8R8A8_UNORM,
            VREND_RES_BIND_SAMPLER_VIEW | VREND_RES_BIND_RENDER_TARGET | (1 << 18),
            300,
            300
        );
#endif

        VirGL::attachResource(current_vgl_ctx, st->surface_info.handle);
        VirGL::attachResource(current_vgl_ctx, WINDOWS_FRAMEBUFFER_HANDLE);

        //2 0x11 1
        st->depth_buffer_info = create3DResource(
            PIPE_TEXTURE_2D,
            VIRGL_FORMAT_S8_UINT_Z24_UNORM,
            VREND_RES_BIND_DEPTH_STENCIL,
            300,
            300
        );
        VirGL::attachResource(current_vgl_ctx, st->depth_buffer_info.handle);

        //  Create Framebuffer 3D object
        st->framebuffer_handle = createSurface(
            st->surface_info.handle,
            VIRGL_FORMAT_B8G8R8A8_UNORM
        );

        st->depth_buffer_handle = createSurface(
            st->depth_buffer_info.handle,
            VIRGL_FORMAT_S8_UINT_Z24_UNORM
        );

        // Unknown resources. Dummy
        st->unknown_res_info_0 = create3DResource(0, 0x40, 0x20000, 8, 1, 1, 0);
        VirGL::attachResource(current_vgl_ctx, st->unknown_res_info_0.handle);
        st->unknown_res_info_2 = create3DResource(0, 0x40, 0x20000, 8, 1, 1, 0);
        VirGL::attachResource(current_vgl_ctx, st->unknown_res_info_2.handle);

        st->DSA_handle = createDSA(0, 0, 0, 0);
        cmd->bindObject(st->DSA_handle, VIRGL_OBJECT_DSA);

        st->frag_shader_handle = createDefaultFragmentShader();
        st->vert_shader_handle = createDefaultVertexShader();
        st->rasterizer_handle = createDefaultRasterizer();
        st->blend_handle = createDefaultBlend();

        setDefaultPolygonStipple(cmd);
        setDefaultViewportState(cmd);
        setDefaultScissorState(cmd);
        setDefaultFramebuffer(cmd, st->framebuffer_handle, st->depth_buffer_handle);
        setDefaultConstantBuffers(cmd);

        TRACE_OUT();
    }

    INT createContext(UINT32 *id)
    {
        TRACE_IN();
        assert(id);

        UINT32 i;
        for (i = 0; i < MAX_STATE_COUNT; i++)
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
        if (id >= MAX_STATE_COUNT)
            return STATE_ERROR_INVALID_ID;
        if (!states[id])
            return STATE_ERROR_INVALID_CONTEXT;

        VirGL::VirglCommandBuffer *cmd = states[current_sub_ctx]->command_buffer;

        cmd->emptyCommandBuffer();

        cmd->deleteSubContext(id);
        if (id != 0)
            cmd->setCurrentSubContext(current_sub_ctx);
        cmd->submitCommandBuffer();

        delete states[id];
        states[id] = NULL;
        current_sub_ctx = 0;

        for (UINT32 i = 0; i < MAX_STATE_COUNT; i++)
            if (states[i])
                return STATUS_SUCCESS;

        deleteVglCtx();
        return STATUS_SUCCESS;
    }
}
