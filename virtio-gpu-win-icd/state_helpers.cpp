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

    CONST CHAR* errorToStr(INT error)
    {
#define ERRTOSTR(Error) case Error: return #Error
        switch (error)
        {
        ERRTOSTR(STATUS_SUCCESS);
        ERRTOSTR(STATE_ERROR_INVALID_ID);
        ERRTOSTR(STATE_ERROR_CONTEXT_EXISTS);
        ERRTOSTR(STATE_ERROR_CANNOT_ALLOC_CONTEXT);
        ERRTOSTR(STATE_ERROR_INVALID_CONTEXT);
        ERRTOSTR(STATE_ERROR_INVALID_CURRENT_CONTEXT);
        ERRTOSTR(STATE_ERROR_NOT_ALLOWED);
        default:
            return "UNKNOWN";
        }
#undef ERRTOSTR
    }

    VirGL::RESOURCE_CREATION create3DResource
        (
            UINT32 target, UINT32 format, UINT32 bind, UINT32 width,
            UINT32 height, UINT32 depth, UINT32 array_size
        )
    {
        UINT32 handle = states[current_sub_ctx]->resource_index;
        states[current_sub_ctx]->resource_index++;

        VirGL::RESOURCE_CREATION info = { 0 };
        info.handle = handle;
        info.target = target;
        info.format = format;
        info.bind = bind;
        info.width = width;
        info.height = height;
        info.depth = depth;
        info.array_size = array_size;

        VirGL::createResource3d(current_vgl_ctx, info);

        return info;
    }

    UINT32 createObject(UINT32 type, std::vector<UINT32>& args)
    {
        UINT32 handle = states[current_sub_ctx]->object_index;
        states[current_sub_ctx]->command_buffer->createObject(handle, type, args);
        states[current_sub_ctx]->object_index += 1;

        return handle;
    }

    UINT32 createSurface(UINT32 res_handle, UINT32 format)
    {
        std::vector<UINT32> args(4);

        args[0] = res_handle;
        args[1] = format;
        args[2] = 0x0;
        args[3] = 0x0;
        return createObject(VIRGL_OBJECT_SURFACE, args);

    }

    UINT32 createDSA(UINT32 a, UINT32 b, UINT32 c, UINT32 d)
    {
        std::vector<UINT32> args(4);

        args[0] = a;
        args[1] = b;
        args[2] = c;
        args[3] = d;
        return createObject(VIRGL_OBJECT_DSA, args);

    }
}
