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

    typedef UINT32 GPU_3D_CMD;
    typedef GPU_3D_CMD* PGPU_3D_CMD;

    struct VirglCommandBuffer {
        std::vector<std::pair<GPU_3D_CMD, std::vector<UINT32>>> m_commands;
        UINT32 m_total_size;
        UINT32 m_ctx_id;

        VirglCommandBuffer(UINT32 vgl_ctx);

        BOOL submitCommandBuffer();

        VOID createSubContext(UINT32 sub_ctx);
        VOID setCurrentSubContext(UINT32 sub_ctx);
        VOID deleteSubContext(UINT32 sub_ctx);

        /* UNTESTED */
        VOID clear(FLOAT rgba[4], double depth, UINT32 stencil);
        VOID setViewportState(FLOAT scale[3], FLOAT translation[3]);
        VOID createObject(UINT32 type, UINT32 handle, UINT32 size);
        VOID setFramebuffer(UINT32 handle, UINT32 zbuff_handle);
        VOID drawVBO(VBO_SETTINGS vbo);
    };

    VOID printHost(const char* message);

    VOID createContext(UINT32 vgl_ctx);
    VOID deleteContext(UINT32 vgl_ctx);
    VOID create_resource_2d(UINT32 ctx_id, UINT32 res_id, UINT32 format, UINT32 width, UINT32 height);
    VOID create_resource_3d(UINT32 ctx_id, RESOURCE_CREATION info);
    VOID attach_resource(UINT32 ctx_id, UINT32 res_id);
}
