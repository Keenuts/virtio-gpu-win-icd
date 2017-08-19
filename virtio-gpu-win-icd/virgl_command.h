#pragma once

#include <array>
#include <vector>
#include <utility>

#include "win_types.h"

namespace VirGL
{
    typedef struct _VBO_SETTINGS {
        UINT32 indexed;
        UINT32 mode;
        UINT32 start;
        UINT32 count;

        UINT32 start_instance;
        UINT32 instance_count;

        INT32 index_bias;
        UINT32 min_index;
        UINT32 max_index;

        UINT32 primitive_restart;
        UINT32 restart_index;
        UINT32 cso;
    } VBO_SETTINGS, *PVBO_SETTINGS;

#define VIRTIO_GPU_RESOURCE_FLAG_Y_0_TOP (1 << 0)
    typedef struct _RESOURCE_CREATION {
        UINT32 handle;
        UINT32 target;
        UINT32 format;
        UINT32 bind;
        UINT32 width;
        UINT32 height;
        UINT32 depth;
        UINT32 array_size;
        UINT32 last_level;
        UINT32 nr_samples;
        UINT32 flags;
    } RESOURCE_CREATION;

    typedef struct _INLINE_WRITE {
        UINT32 handle;
        UINT32 data_len;
        VOID *data;
        UINT32 level;
        UINT32 usage;
        UINT32 stride;
        UINT32 layer_stride;
        UINT32 x;
        UINT32 y;
        UINT32 z;
        UINT32 width;
        UINT32 height;
        UINT32 depth;
    } INLINE_WRITE;

    typedef UINT32 GPU_3D_CMD;
    typedef GPU_3D_CMD* PGPU_3D_CMD;

    struct VirglCommandBuffer {
        std::vector<std::pair<GPU_3D_CMD, std::vector<UINT32>>> m_commands;
        UINT32 m_total_size;
        UINT32 m_ctx_id;

        VirglCommandBuffer(UINT32 vgl_ctx);

        INT submitCommandBuffer();

        VOID createSubContext(UINT32 sub_ctx);
        VOID setCurrentSubContext(UINT32 sub_ctx);
        VOID deleteSubContext(UINT32 sub_ctx);

        /* UNTESTED */
        VOID clear(FLOAT rgba[4], double depth, UINT32 stencil);
        VOID setViewportState(FLOAT scale[3], FLOAT translation[3]);

        VOID createObject(UINT32 handle, UINT32 type, std::vector<UINT32>& args);
        VOID bindObject(UINT32 handle, UINT32 type);
        VOID bindShader(UINT32 handle, UINT32 type);
        VOID setViewportState(UINT32 start_slot, std::vector<FLOAT>& values);
        VOID setPolygonStipple(std::vector<UINT32>& stipple);
        VOID setFramebufferState(UINT32 nb_cbuf, UINT32 zsurf_handle, std::vector<UINT32>& surf_handles);
        VOID setConstantBuffer(UINT32 shader_type, UINT32 index, std::vector<float> constants);

        VOID inlineWrite(INLINE_WRITE data);

        VOID drawVBO(VBO_SETTINGS vbo);

    };

    VOID printHost(const char* message);

    VOID createContext(UINT32 vgl_ctx);
    VOID deleteContext(UINT32 vgl_ctx);

    VOID createResource2d(UINT32 ctx_id, UINT32 res_id, UINT32 format, UINT32 width, UINT32 height);
    VOID createResource3d(UINT32 ctx_id, RESOURCE_CREATION info);
    VOID attachResource(UINT32 ctx_id, UINT32 res_id);

    VOID detachResource(UINT32 ctx_id, UINT32 res_id);
    VOID unrefResource(UINT32 res_id);
}

