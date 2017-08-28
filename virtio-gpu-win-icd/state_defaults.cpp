#include "debug.h"
#include "state.h"
#include "tmp_const.h"
#include "virgl.h"
#include "virgl_command.h"


namespace State
{
    extern UINT32 current_sub_ctx;
    extern UINT32 current_vgl_ctx;
    extern OpenGLState *states[MAX_STATE_COUNT];

    typedef struct _SHADER_INFO {
        UINT32 type;
        UINT32 *tokens;
        UINT32 token_count;
        UINT32 offlen;
        UINT32 num_so_output;
    } SHADER_INFO;

    static UINT32 createShader(SHADER_INFO *info)
    {
        TRACE_IN();

        UINT32 handle, i;
        OpenGLState *st = states[current_sub_ctx];
        VirGL::VirglCommandBuffer *cmd = st->command_buffer;

        const UINT32 nb_cells = (UINT32)_CMATH_::ceil((double)info->token_count / 4.0);
        std::vector<UINT32> create_info(4 + nb_cells);

        create_info[0] = info->type;
        create_info[1] = info->token_count;
        create_info[2] = info->offlen;
        create_info[3] = info->num_so_output;
        for (i = 0; i < nb_cells; i++)
            create_info[i + 4] = info->tokens[i];

        handle = createObject(VIRGL_OBJECT_SHADER, create_info);
        cmd->bindShader(handle, info->type);

        TRACE_OUT();
        return handle;
    }

    UINT32 createDefaultFragmentShader(VOID)
    {
        TRACE_IN();

        UINT32 handle = 0;
        SHADER_INFO info = { 0 };

        info.type = VIRGL_SHADER_TYPE_FRAGMENT;
        info.tokens = TmpConst::shader_frag;
        info.token_count = TmpConst::shader_frag_num_tokens;
        info.offlen = TmpConst::shader_frag_offlen;
        info.num_so_output = TmpConst::shader_frag_num_so_output;

        handle = createShader(&info);

        TRACE_OUT();
        return handle;
    }

    UINT32 createDefaultVertexShader(VOID)
    {
        TRACE_IN();

        UINT32 handle = 0;
        SHADER_INFO info = { 0 };

        info.type = VIRGL_SHADER_TYPE_VERTEX;
        info.tokens = TmpConst::shader_vert;
        info.token_count = TmpConst::shader_vert_num_tokens;
        info.offlen = TmpConst::shader_vert_offlen;
        info.num_so_output = TmpConst::shader_vert_num_so_output;

        handle = createShader(&info);

        TRACE_OUT();
        return handle;
    }

    UINT32 createDefaultRasterizer(VOID)
    {
        TRACE_IN();

        UINT32 handle;
        OpenGLState *st = states[current_sub_ctx];
        VirGL::VirglCommandBuffer *cmd = st->command_buffer;

        UINT32 bitfield1 = 0;
        UINT32 bitfield2 = 0;
        std::vector<UINT32> create_info(8);

#define SET_BIT_AT(Var, Value, Pos) Var = (Var & ~(1 << Pos)) | (Value << Pos)

        SET_BIT_AT(bitfield1, 1, 1); //Enable depth clip
        SET_BIT_AT(bitfield1, 1, 7); //point quad rasterization
        SET_BIT_AT(bitfield1, 1, 8); //Fill front
        SET_BIT_AT(bitfield1, 1, 15); //offset_line
        SET_BIT_AT(bitfield1, 1, 29); // ??
        SET_BIT_AT(bitfield1, 1, 30); // ??

        bitfield2 = 0xffff; //line_stipple_pattern, don't ask me why
#undef SET_BIT_AT

        create_info[0] = bitfield1;
        create_info[1] = 0x3f800000; //point size = 1.0f
        create_info[2] = 0; //Sprit coord enabled ?
        create_info[3] = bitfield2;
        create_info[4] = 0x3f800000; //line width = 1.0f
        create_info[5] = 0; // offset units
        create_info[6] = 0; // offset scale
        create_info[7] = 0; //offset clamp

        handle = createObject(VIRGL_OBJECT_RASTERIZER, create_info);
        cmd->bindObject(handle, VIRGL_OBJECT_RASTERIZER);

        TRACE_OUT();
        return handle;
    }

    UINT32 createDefaultBlend(VOID)
    {
        TRACE_IN();

        UINT32 i, handle;
        OpenGLState *st = states[current_sub_ctx];
        VirGL::VirglCommandBuffer *cmd = st->command_buffer;

        UINT32 bitfield_1 = 0;
        UINT32 bitfield_2 = 0;
        UINT32 bitfield_3 = 0;
        std::vector<UINT32> create_info(10);

        bitfield_1 |= 1 << 2;
        bitfield_3 |= 0xf << 27;

        for (i = 0; i < 10; i++)
            create_info[i] = 0;
        create_info[0] = bitfield_1;
        create_info[1] = bitfield_2;
        create_info[2] = bitfield_3;

        handle = createObject(VIRGL_OBJECT_BLEND, create_info);
        cmd->bindObject(handle, VIRGL_OBJECT_BLEND);

        TRACE_OUT();
        return handle;
    }

    INT setDefaultPolygonStipple(VirGL::VirglCommandBuffer *cmd)
    {
        TRACE_IN();

        std::vector<UINT32> params(32);
        UINT32 i = 0;

        for (i = 0; i < 32; i++)
            params[i] = 0xffffffff;

        cmd->setPolygonStipple(params);

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT setDefaultScissorState(VirGL::VirglCommandBuffer *cmd)
    {
        TRACE_IN();

        std::vector<UINT32> params(32);
        UINT32 i = 0;

        for (i = 0; i < 16; i++) {
            params[i * 2] = 0;
            params[i * 2 + 1] = 300 | (300 << 16);
        }

        cmd->setScissorState(0, params);

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT setDefaultViewportState(VirGL::VirglCommandBuffer *cmd)
    {
        TRACE_IN();

        std::vector<float> params(6);
        UINT32 i = 0;

#define VAL1 +150.0f
#define VAL2 -150.0f
#define VAL3 +0.5f
#define VAL4 +150.0f
#define VAL5 +150.0f
#define VAL6 +0.5f

        params[0] = VAL1;
        params[1] = VAL2;
        params[2] = VAL3;
        params[3] = VAL4;
        params[4] = VAL5;
        params[5] = VAL6;

        cmd->setViewportState(0, params);

        params.resize(6 * 15);
        for (i = 0; i < 15; i++) {
            params[i * 6 + 0] = VAL1;
            params[i * 6 + 1] = VAL2;
            params[i * 6 + 2] = VAL3;
            params[i * 6 + 3] = VAL4;
            params[i * 6 + 4] = VAL5;
            params[i * 6 + 5] = VAL6;
        }

        cmd->setViewportState(1, params);

        //For some reasons, Mesa intializes the 16 scissors using two commands (1 + 15).

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT setDefaultFramebuffer(VirGL::VirglCommandBuffer *cmd,
                              UINT32 framebufferHandle,
                              UINT32 zbufferHandle)
    {
        TRACE_IN();

        std::vector<UINT32> handles(1);
        handles[0] = framebufferHandle;

        cmd->setFramebufferState(1, zbufferHandle, handles);

        TRACE_OUT();
        return STATUS_SUCCESS;
    }

    INT setDefaultConstantBuffers(VirGL::VirglCommandBuffer *cmd)
    {
        TRACE_IN();

        //These values come from an OpenGL app ran on Fedora
        std::vector<FLOAT> payload(sizeof(TmpConst::constant_buffer) / sizeof(FLOAT));
        memcpy(
                &payload.data()[0],
                &TmpConst::constant_buffer[0],
                sizeof(TmpConst::constant_buffer)
        );

        cmd->setConstantBuffer(VIRGL_SHADER_TYPE_VERTEX, 0, payload);

        TRACE_OUT();
        return STATUS_SUCCESS;
    }
}
