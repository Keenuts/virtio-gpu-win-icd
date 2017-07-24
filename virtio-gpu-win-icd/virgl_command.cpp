#include <vector>

#include "debug.h"
#include "driver_api.h"
#include "virgl.h"
#include "virgl_command.h"
#include "win_types.h"

namespace VirGL
{
    static GPU_3D_CMD createHeader(UINT32 command, UINT32 opt, UINT32 length)
    {
        GPU_3D_CMD cmd = { 0 };
        cmd |= command & 0xFF;
        cmd |= (opt & 0xFF) << 8;
        cmd |= length << 16;
        return cmd;
    }

#define LENGTH_FROM_HEADER(Header) (Header >> 16)

    VirglCommandBuffer::VirglCommandBuffer(UINT32 vgl_ctx)
        : m_ctx_id(vgl_ctx), m_total_size(0), m_commands()
    { }

    BOOL VirglCommandBuffer::submitCommandBuffer()
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] submit cmd buffer\n"));

        BYTE *buffer = new BYTE[m_total_size + sizeof(GPU_SUBMIT_3D)];
        if (!buffer)
            return FALSE;

        GPU_SUBMIT_3D header = { 0 };
        header.hdr.type = VIRTIO_GPU_CMD_SUBMIT_3D;
        header.hdr.ctx_id = m_ctx_id;
        header.size = m_total_size;

        memcpy(buffer, &header, sizeof(GPU_SUBMIT_3D));
        UINT32 offset = sizeof(GPU_SUBMIT_3D);

        DbgPrint(TRACE_LEVEL_INFO, ("Creating command buffer of size: %zu\n", m_total_size + sizeof(GPU_SUBMIT_3D)));

        for (std::pair<GPU_3D_CMD, std::vector<UINT32>> cmd : m_commands) {
            GPU_3D_CMD head = cmd.first;
            UINT32 *params = cmd.second.data();

            memcpy(buffer + offset, &head, sizeof(head));
            offset += sizeof(head);
            memcpy(buffer + offset, params, sizeof(UINT32) * LENGTH_FROM_HEADER(head));
            offset += LENGTH_FROM_HEADER(head) * sizeof(UINT32);
        }

        sendCommand(buffer, m_total_size + sizeof(GPU_SUBMIT_3D));
        //For now send command crash on error

        delete[] buffer;

        TRACE_OUT();
        return TRUE;
    };

    VOID VirglCommandBuffer::createSubContext(UINT32 sub_ctx)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] create sub ctx\n"));

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params;

        head = createHeader(VIRGL_CCMD_CREATE_SUB_CTX, 0, 1);
        params.push_back(sub_ctx);

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);
        TRACE_OUT();
    }

    VOID VirglCommandBuffer::setCurrentSubContext(UINT32 sub_ctx)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] set sub context\n"));

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params;

        head = createHeader(VIRGL_CCMD_SET_SUB_CTX, 0, 1);
        params.push_back(sub_ctx);

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);
        TRACE_OUT();
    }

    VOID VirglCommandBuffer::deleteSubContext(UINT32 sub_ctx)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Delete sub ctx\n"));

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params;

        head = createHeader(VIRGL_CCMD_DESTROY_SUB_CTX, 0, 1);
        params.push_back(sub_ctx);

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

        TRACE_OUT();
    }


    VOID VirglCommandBuffer::clear(FLOAT rgba[4], double depth, UINT32 stencil)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Clear\n"));

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params;

        head = createHeader(VIRGL_CCMD_CLEAR, 0, 8);
        UINT32 buf_idx = 0;

        params.resize(8);
        memcpy(params.data(), &buf_idx, sizeof(UINT32));
        memcpy(params.data() + 1, rgba, sizeof(FLOAT) * 4);
        memcpy(params.data() + 5, &depth, sizeof(UINT64));
        memcpy(params.data() + 7, &stencil, sizeof(UINT32));

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::setViewportState(FLOAT scale[3], FLOAT translation[3])
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] SetViewport\n"));

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params((sizeof(FLOAT) / sizeof(UINT32)) * 6);

        const UINT32 length = (sizeof(FLOAT) / sizeof(UINT32)) * 6;
        head = createHeader(VIRGL_CCMD_SET_VIEWPORT_STATE, 0, length);


        memcpy(params.data(), scale, sizeof(FLOAT) * 3);
        memcpy(params.data() + (sizeof(FLOAT) * 3), translation, sizeof(FLOAT) * 3);

        m_commands.emplace_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::createObject(UINT32 type, UINT32 handle, UINT32 size)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Create object\n"));

        UNREFERENCED_PARAMETER(type);
        UNREFERENCED_PARAMETER(handle);
        UNREFERENCED_PARAMETER(size);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::setFramebuffer(UINT32 handle, UINT32 zbuff_handle)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] set framebuffer\n"));

        UNREFERENCED_PARAMETER(handle);
        UNREFERENCED_PARAMETER(zbuff_handle);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::drawVBO(VBO_SETTINGS vbo)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Draw VBO\n"));

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
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Create context\n"));

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
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Delete context\n"));

        GPU_CTX_DESTROY cmd = { 0 };
        cmd.hdr.type = VIRTIO_GPU_CMD_CTX_DESTROY;
        cmd.hdr.ctx_id = vgl_ctx;

        sendCommand(&cmd, sizeof(cmd));

        TRACE_OUT();
    }

    VOID create_resource_2d(UINT32 ctx_id, UINT32 res_id, UINT32 format, UINT32 width, UINT32 height)
    {
        TRACE_IN();

        GPU_RES_CREATE_2D info = { 0 };
        info.format = format;
        info.height = height;
        info.width = width;
        info.resource_id = res_id;
        info.hdr.ctx_id = ctx_id;
        info.hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;

        sendCommand(&info, sizeof(GPU_RES_CREATE_2D));

        TRACE_OUT();
    }

    VOID create_resource_3d(UINT32 ctx_id, RESOURCE_CREATION info)
    {
        TRACE_IN();

        GPU_RES_CREATE_3D data = { 0 };
        
        data.hdr.ctx_id = ctx_id;
        data.hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_3D;
        data.resource_id = info.handle;
        data.target = info.target;
        data.format = info.format;
        data.bind = info.bind;
        data.width = info.width;
        data.height = info.height;
        data.depth = info.depth;
        data.array_size = info.array_size;
        data.last_level = info.last_level;
        data.nr_samples = info.nr_samples;
        data.flags = info.flags;
        sendCommand(&data, sizeof(GPU_RES_CREATE_3D));

    VOID allocate_object(UINT32 size, UINT64 *handle);
    VOID update_object(UINT64 handle, VOID *data, UINT64 size);
    VOID destroy_object(UINT64 handle);
        TRACE_OUT();
    }

    VOID attach_backing(UINT32 ctx_id, UINT32 res_id, UINT32 nr_entries)
    {
        TRACE_IN();

        GPU_RES_ATTACH_BACKING info = { 0 };
        info.hdr.ctx_id = ctx_id;
        info.hdr.type = VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE;
        info.nr_entries = nr_entries;
        info.resource_id = res_id;

        sendCommand(&info, sizeof(info));

        TRACE_OUT();
    }

    VOID attach_resource(UINT32 ctx_id, UINT32 res_id)
    {
        TRACE_IN();

        GPU_RES_ATTACH info = { 0 };
        info.hdr.ctx_id = ctx_id;
        info.hdr.type = VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE;
        info.resource_id = res_id;
        sendCommand(&info, sizeof(info));

        TRACE_OUT();
    }
}