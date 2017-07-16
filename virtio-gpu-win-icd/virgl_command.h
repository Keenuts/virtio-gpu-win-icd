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

    typedef struct _GPU_3D_CMD {
        UINT32 command : 8;
        UINT32 opt : 8;
        UINT32 length : 16;
    } GPU_3D_CMD, *PGPU_3D_CMD;

    struct VirglCommandBuffer {
        std::vector<std::pair<GPU_3D_CMD, std::vector<UINT32>>> m_commands;
        UINT32 m_total_size;
        UINT32 m_ctx_id;

        VirglCommandBuffer(UINT32 ctx_id);

        BOOL submitCommandBuffer();

        VOID createSubContext(UINT32 sub_ctx);
        VOID setCurrentSubContext(UINT32 sub_ctx);
        VOID deleteSubContext(UINT32 sub_ctx);

        VOID clear(UINT32 rgba[4], UINT64 depth, UINT32 stencil);
        VOID setViewportState(FLOAT scale[3], FLOAT translation[3]);
        VOID createObject(UINT32 type, UINT32 handle, UINT32 size);
        VOID setFramebuffer(UINT32 handle, UINT32 zbuff_handle);
        VOID drawVBO(VBO_SETTINGS vbo);
    };

    VOID printHost(const char* message);

    VOID createContext(UINT32 vgl_ctx);
    VOID deleteContext(UINT32 vgl_ctx);
}
