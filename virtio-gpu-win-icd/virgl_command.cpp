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

    INT VirglCommandBuffer::submitCommandBuffer()
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] submit cmd buffer\n"));

        BYTE *buffer = new BYTE[m_total_size + sizeof(GPU_SUBMIT_3D)];
        if (!buffer)
            return STATUS_INVALID_PARAMETER;

        GPU_SUBMIT_3D header = { 0 };
        header.hdr.type = VIRTIO_GPU_CMD_SUBMIT_3D;
        header.hdr.ctx_id = m_ctx_id;
        header.size = m_total_size;

        memcpy(buffer, &header, sizeof(GPU_SUBMIT_3D));
        UINT32 offset = sizeof(GPU_SUBMIT_3D);

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
        return STATUS_SUCCESS;
    };

    VOID VirglCommandBuffer::createSubContext(UINT32 sub_ctx)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] create sub ctx\n"));

        GPU_3D_CMD head = { 0 };
        std::vector<UINT32> params;

        head = createHeader(VIRGL_CCMD_CREATE_SUB_CTX, 0, 1);
        params.push_back(sub_ctx);

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
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

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
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

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
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

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::setViewportState(FLOAT scale[3], FLOAT translation[3])
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] SetViewport\n"));

        GPU_3D_CMD head = { 0 };
        const UINT32 length = (sizeof(FLOAT) / sizeof(UINT32)) * 6;
        std::vector<UINT32> params(length);

        head = createHeader(VIRGL_CCMD_SET_VIEWPORT_STATE, 0, length);


        memcpy(params.data(), scale, sizeof(FLOAT) * 3);
        memcpy(params.data() + (sizeof(FLOAT) * 3), translation, sizeof(FLOAT) * 3);

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::createObject(UINT32 handle, UINT32 type, std::vector<UINT32>& args)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Create object 0x%x of type 0x%x\n", handle, type));

        GPU_3D_CMD head = { 0 };
        const UINT32 length = 1 + (UINT32)args.size();

        std::vector<UINT32> params(length);
        head = createHeader(VIRGL_CCMD_CREATE_OBJECT, type, length);
        params[0] = handle;
        for (UINT32 i = 0; i < args.size(); i++)
            params[i + 1] = args[i];

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::bindObject(UINT32 handle, UINT32 type)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Binding object 0x%x to type 0x%x\n", handle, type));

        GPU_3D_CMD head = { 0 };
        const UINT32 length = 1;
        std::vector<UINT32> params(length);
        head = createHeader(VIRGL_CCMD_BIND_OBJECT, type, length);
        params[0] = handle;

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::bindShader(UINT32 handle, UINT32 type)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Binding a shader 0x%x of type 0x%x\n", handle, type));

        GPU_3D_CMD head = { 0 };
        const UINT32 length = 2;
        std::vector<UINT32> params(length);

        head = createHeader(VIRGL_CCMD_BIND_SHADER, 0, length);
        params[0] = handle;
        params[1] = type;

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

        TRACE_OUT();
    }

	VOID VirglCommandBuffer::setViewportState(UINT32 start_slot, std::vector<FLOAT>& values)
	{
		TRACE_IN();

		assert(values.size() != 0);
		assert(values.size() % 6 == 0);

        DbgPrint(TRACE_LEVEL_INFO, ("[?] Viewport setting for %u viewports\n", (UINT32)(values.size() / 6)));

		GPU_3D_CMD head = { 0 };
		const UINT32 length = 1 + (UINT32)values.size();
		std::vector<UINT32> params(length);

		head = createHeader(VIRGL_CCMD_SET_VIEWPORT_STATE, 0, length);
		params[0] = start_slot;
		for (UINT32 i = 0; i < (UINT32)values.size(); i++)
			params[i + 1] = (UINT32)values[i];

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

		TRACE_OUT();
	}

	VOID VirglCommandBuffer::setFramebufferState(UINT32 nb_cbuf, UINT32 zsurf_handle, std::vector<UINT32>& surf_handles)
	{
		TRACE_IN();

		assert(surf_handles.size() == nb_cbuf);
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Franebuffer setting for %u viewports\n", (UINT32)surf_handles.size()));

		GPU_3D_CMD head = { 0 };
		const UINT32 length = 2 + (UINT32)surf_handles.size();
		std::vector<UINT32> params(length);

		head = createHeader(VIRGL_CCMD_SET_FRAMEBUFFER_STATE, 0, length);
		params[0] = nb_cbuf;
		params[1] = zsurf_handle;
		for (UINT32 i = 0; i < (UINT32)surf_handles.size(); i++)
			params[i + 2] = (UINT32)surf_handles[i];

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

		TRACE_OUT();
	}

	VOID VirglCommandBuffer::setConstantBuffer(UINT32 shader_type, UINT32 index, std::vector<float> constants)
	{
        TRACE_IN();

        DbgPrint(TRACE_LEVEL_INFO, ("[?] Set constant buffer for shader type %d\n", shader_type));

		GPU_3D_CMD head = { 0 };
		const UINT32 length = 2 + (UINT32)constants.size();
		std::vector<UINT32> params(length);

		head = createHeader(VIRGL_CCMD_SET_CONSTANT_BUFFER, 0, length);
		params[0] = shader_type;
		params[1] = index;
		for (UINT32 i = 0; i < (UINT32)constants.size(); i++)
			params[i + 2] = (UINT32)constants[i];

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

		TRACE_OUT();
	}

    VOID VirglCommandBuffer::inlineWrite(INLINE_WRITE info)
    {
        TRACE_IN();

        DbgPrint(TRACE_LEVEL_INFO, ("[?] Inline write\n"));

        GPU_3D_CMD head = { 0 };
        const UINT32 length = 11 + info.data_len / sizeof(UINT32);
        std::vector<UINT32> params(length);

        head = createHeader(VIRGL_CCMD_RESOURCE_INLINE_WRITE, 0, length);
        params[0] = info.handle;
        params[1] = info.level;
        params[2] = info.usage;
        params[3] = info.stride;
        params[4] = info.layer_stride;
        params[5] = info.x;
        params[6] = info.y;
        params[7] = info.z;
        params[8] = info.width;
        params[9] = info.height;
        params[10] = info.depth;

        UINT32 *ptr = (UINT32*)info.data;
        for (UINT32 i = 0; i < length - 11; i++)
            params[11 + i] = ptr[i];

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

        TRACE_OUT();
    }

    VOID VirglCommandBuffer::drawVBO(VBO_SETTINGS vbo)
    {
        TRACE_IN();

        DbgPrint(TRACE_LEVEL_INFO, ("[?] Drawing a VBO\n"));

		GPU_3D_CMD head = { 0 };
		const UINT32 length = 12;
		std::vector<UINT32> params(length);

		head = createHeader(VIRGL_CCMD_DRAW_VBO, 0, length);
		params[0] = vbo.start;
		params[1] = vbo.count;
		params[2] = vbo.mode;
		params[3] = vbo.indexed;
		params[4] = vbo.instance_count;
		params[5] = vbo.index_bias;
		params[6] = vbo.start_instance;
		params[7] = vbo.primitive_restart;
		params[8] = vbo.restart_index;
		params[9] = vbo.min_index;
		params[10] = vbo.max_index;
		params[11] = vbo.cso;

        m_commands.push_back(std::pair<GPU_3D_CMD, std::vector<UINT32>>(head, params));
        m_total_size += sizeof(head) + sizeof(UINT32) * LENGTH_FROM_HEADER(head);

		TRACE_OUT();
    }

    VOID printHost(const char* message)
    {
        TRACE_IN();

        PGPU_SHOWDEBUG cmd = new GPU_SHOWDEBUG();
        assert(cmd);
        memset(cmd, 0, sizeof(GPU_SHOWDEBUG));

        cmd->hdr.type = VIRTIO_GPU_CMD_SHOW_DEBUG;
        cmd->hdr.ctx_id = 0;

        for (UINT32 i = 0; message[i] && i < SHOWDEBUG_SIZE; i++)
            cmd->message[i] = message[i];
        cmd->message[SHOWDEBUG_SIZE - 1] = 0;


        // sendCommand(cmd, sizeof(GPU_SHOWDEBUG));
        DbgPrint(TRACE_LEVEL_WARNING, ("[!] Print on host has been disabled.\n"));
        UNREFERENCED_PARAMETER(cmd);

        delete cmd;

        TRACE_OUT();
    }

    VOID createContext(UINT32 vgl_ctx)
    {
        TRACE_IN();
        DbgPrint(TRACE_LEVEL_INFO, ("[?] Create context\n"));

#if 1
        GPU_CTX_CREATE cmd = { 0 };
        cmd.hdr.type = VIRTIO_GPU_CMD_CTX_CREATE;
        cmd.hdr.ctx_id = vgl_ctx;
        cmd.nlen = 0;

        sendCommand(&cmd, sizeof(cmd));
#else
        UNREFERENCED_PARAMETER(vgl_ctx);
        DbgPrint(TRACE_LEVEL_ERROR, ("BUG BUSTER IN\n"));
#define LENGTH 500
        UINT32 *data = new UINT32[LENGTH];
        if (data)
            DbgPrint(TRACE_LEVEL_ERROR, ("Buffer not null\n"));
        sendCommand((VOID*)data, sizeof(*data) * LENGTH);
#undef LENGTH
        DbgPrint(TRACE_LEVEL_ERROR, ("BUG BUSTER OUT\n"));
        assert(0);
#endif

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

    VOID createResource2d(UINT32 ctx_id, UINT32 res_id, UINT32 format, UINT32 width, UINT32 height)
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

    VOID createResource3d(UINT32 ctx_id, RESOURCE_CREATION info)
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

        TRACE_OUT();
    }

    VOID attachBacking(UINT32 ctx_id, UINT32 res_id, UINT32 nr_entries)
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

    VOID attachResource(UINT32 ctx_id, UINT32 res_id)
    {
        TRACE_IN();

        GPU_RES_ATTACH info = { 0 };
        info.hdr.ctx_id = ctx_id;
        info.hdr.type = VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE;
        info.resource_id = res_id;
        sendCommand(&info, sizeof(info));

        TRACE_OUT();
    }

    VOID detachResource(UINT32 ctx_id, UINT32 res_id)
    {
        TRACE_IN();

        GPU_RES_DETACH info = { 0 };
        info.hdr.ctx_id = ctx_id;
        info.hdr.type = VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE;
        info.resource_id = res_id;
        sendCommand(&info, sizeof(info));

        TRACE_OUT();
    }

    VOID unrefResource(UINT32 res_id)
    {
        TRACE_IN();

        GPU_RES_UNREF info = { 0 };
        info.hdr.type = VIRTIO_GPU_CMD_RESOURCE_UNREF;
        info.resource_id = res_id;
        sendCommand(&info, sizeof(info));
        
        TRACE_OUT();
    }
}