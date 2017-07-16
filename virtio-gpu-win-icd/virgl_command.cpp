#include <vector>

#include "debug.h"
#include "driver_api.h"
#include "virgl.h"
#include "virgl_command.h"
#include "win_types.h"

namespace VirGL
{
    VirglCommandBuffer::VirglCommandBuffer(UINT32 ctx_id)
        : m_ctx_id(ctx_id), m_total_size(0), m_commands()
    { }

    BOOL VirglCommandBuffer::submitCommandBuffer()
    {
        TRACE_IN();

        printHost("[?] Will submit a command buffer\n");
        BYTE *buffer = new BYTE[m_total_size + sizeof(GPU_SUBMIT_3D)];
        if (!buffer)
            return FALSE;

        GPU_SUBMIT_3D header = { 0 };
        header.hdr.type = VIRTIO_GPU_CMD_SUBMIT_3D;
        header.hdr.ctx_id = m_ctx_id;
        header.size = m_total_size / sizeof(UINT32);

        memcpy(buffer, &header, sizeof(GPU_SUBMIT_3D));
        UINT32 offset = sizeof(GPU_SUBMIT_3D);

        DbgPrint(TRACE_LEVEL_INFO, ("Creating command buffer of size: %zu\n", m_total_size + sizeof(GPU_SUBMIT_3D)));

        for (std::pair<GPU_3D_CMD, std::vector<UINT32>> cmd : m_commands) {
            GPU_3D_CMD head = cmd.first;
            UINT32 *params = cmd.second.data();

            memcpy(buffer + offset, &head, sizeof(head));
            offset += 1;
            memcpy(buffer + offset, params, sizeof(UINT32) * head.length);
            offset += head.length;
        }

        sendCommand(buffer, m_total_size);
        //For now send command crash on error

        delete[] buffer;

        TRACE_OUT();
        return TRUE;
    };

    VOID VirglCommandBuffer::createSubContext(UINT32 sub_ctx)
    {
        TRACE_IN();

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params;

        head.length = 1;
        head.command = VIRGL_CCMD_CREATE_SUB_CTX;
        head.opt = 0;
        params.push_back(sub_ctx);

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        TRACE_OUT();
    }

    VOID VirglCommandBuffer::setCurrentSubContext(UINT32 sub_ctx)
    {
        TRACE_IN();

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params;

        head.length = 1;
        head.command = VIRGL_CCMD_SET_SUB_CTX;
        head.opt = 0;
        params.push_back(sub_ctx);

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        TRACE_OUT();
    }

    VOID VirglCommandBuffer::deleteSubContext(UINT32 sub_ctx)
    {
        TRACE_IN();

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params;

        head.length = 1;
        head.command = VIRGL_CCMD_DESTROY_SUB_CTX;
        head.opt = 0;
        params.push_back(sub_ctx);

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));

        TRACE_OUT();
    }


    VOID VirglCommandBuffer::clear(UINT32 rgba[4], UINT64 depth, UINT32 stencil)
    {
        TRACE_IN();

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params;

        head.length = 7;
        head.command = VIRGL_CCMD_CLEAR;
        head.opt = 0;

        params.push_back(0); //FIXME: buffer index
        for (UINT32 i = 0; i < 4; i++)
            params.push_back(rgba[i]);
        //Space for the UINT64
        params.push_back(0);
        params.push_back(0);
        params.push_back(stencil);

        memcpy(params.data() + (sizeof(UINT32) * (1 + 4)), &depth, sizeof(UINT64));

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::setViewportState(FLOAT scale[3], FLOAT translation[3])
    {
        TRACE_IN();

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params((sizeof(FLOAT) / sizeof(UINT32)) * 6);

        head.length = (sizeof(FLOAT) / sizeof(UINT32)) * 6;
        head.command = VIRGL_CCMD_SET_VIEWPORT_STATE;
        head.opt = 0;


        memcpy(params.data(), scale, sizeof(FLOAT) * 3);
        memcpy(params.data() + (sizeof(FLOAT) * 3), translation, sizeof(FLOAT) * 3);

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::createObject(UINT32 type, UINT32 handle, UINT32 size)
    {
        TRACE_IN();

        UNREFERENCED_PARAMETER(type);
        UNREFERENCED_PARAMETER(handle);
        UNREFERENCED_PARAMETER(size);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::setFramebuffer(UINT32 handle, UINT32 zbuff_handle)
    {
        TRACE_IN();

        UNREFERENCED_PARAMETER(handle);
        UNREFERENCED_PARAMETER(zbuff_handle);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::drawVBO(VBO_SETTINGS vbo)
    {
        TRACE_IN();

        UNREFERENCED_PARAMETER(vbo);

        TRACE_OUT();
    }

    VOID printHost(const char* message)
    {
        TRACE_IN();

        GPU_SHOWDEBUG cmd = { 0 };
        cmd.hdr.type = VIRTIO_GPU_CMD_SHOW_DEBUG;
        cmd.hdr.ctx_id = 0;
        strncpy_s(cmd.message, message, SHOWDEBUG_SIZE);

        sendCommand(&cmd, sizeof(cmd));

        TRACE_OUT();
    }

    VOID createContext(UINT32 vgl_ctx)
    {
        TRACE_IN();

        printHost("[?] Creating context\n");
        GPU_CTX_CREATE cmd = { 0 };
        cmd.hdr.type = VIRTIO_GPU_CMD_CTX_CREATE;
        cmd.hdr.ctx_id = vgl_ctx;
        cmd.nlen = 0;

        sendCommand(&cmd, sizeof(cmd));

        TRACE_OUT();
    }

    VOID deleteContext(UINT32 vgl_ctx)
    {
        TRACE_IN();

        printHost("[?] Deleting context\n");
        GPU_CTX_DESTROY cmd = { 0 };
        cmd.hdr.type = VIRTIO_GPU_CMD_CTX_DESTROY;
        cmd.hdr.ctx_id = vgl_ctx;

        sendCommand(&cmd, sizeof(cmd));

        TRACE_OUT();
    }
}