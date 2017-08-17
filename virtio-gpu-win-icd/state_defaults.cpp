#include "debug.h"
#include "state.h"
#include "virgl.h"
#include "virgl_command.h"


namespace State
{
    extern UINT32 current_sub_ctx;
    extern UINT32 current_vgl_ctx;
    extern OpenGLState *states[MAX_STATE_COUNT];
    /*
    static INT loadShader(UINT32 handle, SHADER_INFO shader_info)
    {
        TRACE_IN();

        assert(shader_info.tokens);
        assert(shader_info.token_count);

        BOOL res = 0;
        VirGL::RESOURCE_CREATION info = { 0 };
        info.format = 0x40;
        info.array_size = 1;
        info.bind = shader_info.binding;
        info.depth = 1;
        //FIXME: proper size managent ?
        info.width = 65536;
        info.height = 1;
        info.handle = handle;

        VirGL::createResource3d(current_vgl_ctx, info);
        VirGL::attachResource(current_vgl_ctx, handle);

        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);

        UINT32 nb_cells = (UINT32)_CMATH_::ceil((double)shader_info.token_count / 4.0);
        DbgPrint(TRACE_LEVEL_WARNING, ("Creating a shader contained in %d cells\n", nb_cells));

        std::vector<UINT32> create_info(4 + nb_cells);
        create_info[0] = shader_info.type;
        create_info[1] = shader_info.token_count;
        create_info[2] = shader_info.offlen;
        create_info[3] = shader_info.num_so_output;
        for (UINT32 i = 0; i < nb_cells; i++)
            create_info[i + 4] = shader_info.tokens[i];

        cmd.createObject(handle, VIRGL_OBJECT_SHADER, create_info);

        cmd.bindShader(handle, shader_info.type);
        res = cmd.submitCommandBuffer();

        if (res != STATUS_SUCCESS)
            DbgPrint(TRACE_LEVEL_ERROR, ("[!] Unable to load the shader id=0x%x\n", handle));
        assert(res == STATUS_SUCCESS);

        TRACE_OUT();
        return STATUS_SUCCESS;
    }
    */

    INT createDefaultFragmentShader(VOID)
    {
        TRACE_IN();

        UINT32 handle = DEFAULT_FRAG_HANDLE;

        VirGL::RESOURCE_CREATION info = { 0 };
        info.handle = handle;
        info.target = 0;
        info.format = 64;
        info.bind = 0x10;
        info.width = 0x10000;
        info.height = 1;
        info.depth = 1;
        info.array_size = 1;

        VirGL::createResource3d(current_vgl_ctx, info);
        VirGL::attachResource(current_vgl_ctx, handle);

        states[current_sub_ctx]->frag_shader_info = new VirGL::RESOURCE_CREATION();
        memcpy(states[current_sub_ctx]->frag_shader_info, &info, sizeof(info));

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT createDefaultVertexShader(VOID)
    {
        TRACE_IN();

        UINT32 handle = DEFAULT_VERT_HANDLE;

        VirGL::RESOURCE_CREATION info;
        memset(&info, 0, sizeof(VirGL::RESOURCE_CREATION));
        info.handle = handle;
        info.target = 2;
        info.format = 1;
        info.bind = 0x4000a;
        info.width = 1024;
        info.height = 768;
        info.depth = 1;
        info.array_size = 1;

        VirGL::createResource3d(current_vgl_ctx, info);
        VirGL::attachResource(current_vgl_ctx, handle);

        states[current_sub_ctx]->vert_shader_info = new VirGL::RESOURCE_CREATION();
        memcpy(states[current_sub_ctx]->vert_shader_info, &info, sizeof(info));

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT createDefaultRasterizer(VOID)
    {
        TRACE_IN();

        VirGL::RESOURCE_CREATION info;
        memset(&info, 0, sizeof(VirGL::RESOURCE_CREATION));
        info.handle = DEFAULT_RASTERIZER_HANDLE;
        info.target = 2;
        info.format = 2;
        info.bind = 2;
        info.width = 0x40;
        info.height = 0x40;
        info.depth = 1;
        info.array_size = 1;
        info.flags = VIRTIO_GPU_RESOURCE_FLAG_Y_0_TOP;

        VirGL::createResource2d(current_vgl_ctx, DEFAULT_RASTERIZER_HANDLE, 0x2, 0x40, 0x40);

        states[current_sub_ctx]->rasterizer_info = new VirGL::RESOURCE_CREATION();
        memcpy(states[current_sub_ctx]->rasterizer_info, &info, sizeof(info));

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT createDefaultBlend(VOID)
    {
        TRACE_IN();

        VirGL::RESOURCE_CREATION info;
        memset(&info, 0, sizeof(VirGL::RESOURCE_CREATION));
        info.handle = DEFAULT_BLEND_HANDLE;
        info.target = 2;
        info.format = 0xb1;
        info.bind = 0xa;
        info.width = 0x268;
        info.height = 0x1f;
        info.depth = 1;
        info.array_size = 1;

        VirGL::createResource3d(current_vgl_ctx, info);
        VirGL::attachResource(current_vgl_ctx, info.handle);

        states[current_sub_ctx]->blend_info = new VirGL::RESOURCE_CREATION();
        memcpy(states[current_sub_ctx]->blend_info, &info, sizeof(info));

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT createDefaultVertexElements(VOID)
    {
        TRACE_IN();

        VirGL::RESOURCE_CREATION info;
        memset(&info, 0, sizeof(VirGL::RESOURCE_CREATION));
        info.handle = DEFAULT_VERTEX_ELEMENTS_HANDLE;
        info.target = 0;
        info.format = 0x40;
        info.bind = 0x10;
        info.width = 0x80000;
        info.height = 1;
        info.depth = 1;
        info.array_size = 1;

        VirGL::createResource3d(current_vgl_ctx, info);
        VirGL::attachResource(current_vgl_ctx, info.handle);

        states[current_sub_ctx]->vertex_elements_info = new VirGL::RESOURCE_CREATION();
        memcpy(states[current_sub_ctx]->vertex_elements_info, &info, sizeof(info));

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT loadDefaultRasterizer(VOID)
    {
        TRACE_IN();
        UINT32 handle = DEFAULT_RASTERIZER_HANDLE;

        BOOL res = 0;
        VirGL::RESOURCE_CREATION info;
        memset(&info, 0, sizeof(VirGL::RESOURCE_CREATION));
        info.array_size = 1;
        info.bind = 0x20000;
        info.depth = 1;
        info.width = 8;
        info.height = 1;
        info.handle = handle;
        info.format = 40;

        VirGL::createResource3d(current_vgl_ctx, info);
        VirGL::attachResource(current_vgl_ctx, handle);

        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);

#define SET_BIT_AT(Var, Value, Pos) Var = (Var & ~(1 << Pos)) | (Value << Pos)

        UINT32 bitfield1 = 0;
        UINT32 bitfield2 = 0;
        SET_BIT_AT(bitfield1, 1, 1); //Enable depth clip
        SET_BIT_AT(bitfield1, 1, 7); //point quad rasterization
        SET_BIT_AT(bitfield1, 1, 9); //fill front
        SET_BIT_AT(bitfield1, 1, 15); //offset line
        SET_BIT_AT(bitfield1, 1, 29); // ??
        SET_BIT_AT(bitfield1, 1, 30); // ??

        bitfield2 = 0xffff; //line_stipple_pattern, don't ask me why
#undef SET_BIT_AT

        DbgPrint(TRACE_LEVEL_ERROR, ("Value Bitfield1: 0x%x\n", bitfield1));
        DbgPrint(TRACE_LEVEL_ERROR, ("Value Bitfield2: 0x%x\n", bitfield2));

        std::vector<UINT32> create_info(8);
        create_info[0] = bitfield1;
        create_info[1] = 0x3f800000; //point size = 1.0f
        create_info[2] = 0; //Sprit coord enabled ?
        create_info[3] = bitfield2;
        create_info[3] = 0x3f800000; //line width = 1.0f
        create_info[5] = 0; // offset units
        create_info[6] = 0; // offset scale
        create_info[7] = 0; //offset clamp

        cmd.createObject(handle, VIRGL_OBJECT_RASTERIZER, create_info);
        cmd.bindObject(handle, VIRGL_OBJECT_RASTERIZER);

        res = cmd.submitCommandBuffer();

        if (res != STATUS_SUCCESS)
            DbgPrint(TRACE_LEVEL_ERROR, ("[!] Unable to load default Rasterizer %s(0x%x)", errorToStr(res), res));
        assert(res == STATUS_SUCCESS);

        DbgPrint(TRACE_LEVEL_INFO, ("[?] Default rasterizer loaded\n"));
        states[current_sub_ctx]->blend_id = handle;

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT SetupDefaultBlend(VOID)
    {
        TRACE_IN();

        BOOL res = 0;
        VirGL::RESOURCE_CREATION info;
        VirGL::VirglCommandBuffer cmd(current_vgl_ctx);

        UINT32 handle = DEFAULT_BLEND_HANDLE;
        UINT32 bitfield_1 = 0;
        UINT32 bitfield_2 = 0;
        UINT32 bitfield_3 = 0;
        std::vector<UINT32> create_info(11);

        memset(&info, 0, sizeof(VirGL::RESOURCE_CREATION));
        info.target = 2;
        info.array_size = 1;
        info.bind = 0xa;
        info.depth = 1;
        info.width = 616;
        info.height = 31;
        info.handle = handle;
        info.format = 0xb1;

        VirGL::createResource3d(current_vgl_ctx, info);
        VirGL::attachResource(current_vgl_ctx, handle);

        bitfield_1 |= 1 << 2;
        bitfield_3 |= 0xf << 27;

        for (UINT32 i = 0; i < 11; i++)
            create_info[i] = 0;
        create_info[0] = handle;
        create_info[1] = bitfield_1;
        create_info[2] = bitfield_2;
        create_info[3] = bitfield_3;

        cmd.createObject(handle, VIRGL_OBJECT_BLEND, create_info);
        cmd.bindObject(handle, VIRGL_OBJECT_BLEND);
        res = cmd.submitCommandBuffer();

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    /*
    UINT32 handle = DEFAULT_FRAG_HANDLE;
    SHADER_INFO info = { 0 };
    info.binding = 262154;
    info.num_so_output = TmpConst::shader_frag_num_so_output;
    info.token_count = TmpConst::shader_frag_num_tokens;
    info.offlen = TmpConst::shader_frag_offlen;
    info.tokens = TmpConst::shader_frag;
    info.type = FRAG_SHADER_TYPE;

    INT res = loadShader(handle, info);

    if (res != STATUS_SUCCESS)
        DbgPrint(TRACE_LEVEL_ERROR, ("[!] Unable to load default frag %s(0x%x)", errorToStr(res), res));
    assert(res == STATUS_SUCCESS);

    DbgPrint(TRACE_LEVEL_INFO, ("[?] Default fragment shader loaded\n"));
    states[current_sub_ctx]->frag_shader_id = handle;
    */
}