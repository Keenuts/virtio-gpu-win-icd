#include <cassert>

#include "debug.h"
#include "tests.h"
#include "virgl.h"
#include "virgl_command.h"
#include "state.h"
#include "win_types.h"

#define TRANSFER_FUNCTION "D3DKMTEscape"

struct device_info_t {
	D3DKMT_HANDLE adapter;
    PFND3DKMT_ESCAPE escape;
};

struct gpu_allocate_object_t {
    UINT32 driver_cmd;
    UINT32 size;
    UINT64 handle;
};

struct gpu_update_object_t {
    UINT32 driver_cmd;
    UINT64 handle;
    UINT32 size;
    VOID* ptr;
};

struct gpu_delete_object_t {
    UINT32 driver_cmd;
    UINT64 handle;
};

enum driver_cmd {
    DRIVER_CMD_INVALID = 0,
    DRIVER_CMD_TRANSFER = 1,
    DRIVER_CMD_ALLOCATE,
    DRIVER_CMD_UPDATE,
    DRIVER_CMD_FREE,
};

template<typename PFUNC>
PFUNC getGDIFunction(LPCSTR procName)
{
	static HMODULE lib = NULL;
	if (lib == NULL)
		lib = LoadLibrary("gdi32.dll");
	return reinterpret_cast<PFUNC>(GetProcAddress(lib, procName));
}

#define TOSTR(Status) \
case Status:          \
    return #Status    \

static const char* status2str(NTSTATUS status)
{
    switch (status)
    {
    TOSTR(STATUS_SUCCESS);
    TOSTR(STATUS_INVALID_HANDLE);
    TOSTR(STATUS_INVALID_PARAMETER);
    TOSTR(STATUS_NO_MEMORY);
    TOSTR(STATUS_PRIVILEGED_INSTRUCTION);
    TOSTR(STATUS_ILLEGAL_INSTRUCTION);
    TOSTR(STATUS_GRAPHIC_DRIVER_MISMATCH);
    case 0xC00000BB:
        return "STATUS_NOT_SUPPORTED";
    default:
        return "NO_STRING_FOR_THIS_CODE";
    }
}

static device_info_t initialize_device() {

    static device_info_t info;
    static BOOL initialized = FALSE;

	NTSTATUS res;
	D3DKMT_ENUMADAPTERS adapters;
	PFND3DKMT_ENUMADAPTERS enum_adapter;
    PFND3DKMT_ESCAPE escape;

    if (initialized)
        return info;

    initialized = TRUE;
    if (!Tests::test_enabled) {
        FILE *com_fd;
        assert(freopen_s(&com_fd, "COM2:", "w", stdout) == 0);
    }

    DbgPrint(TRACE_LEVEL_INFO, ("[?] Starting ICD Build on %s %s: .\n", __DATE__, __TIME__));

	enum_adapter = getGDIFunction<PFND3DKMT_ENUMADAPTERS>("D3DKMTEnumAdapters");
	escape = getGDIFunction<PFND3DKMT_ESCAPE>("D3DKMTEscape");
    assert(enum_adapter);
    assert(escape);

	memset(&adapters, 0, sizeof(D3DKMT_ENUMADAPTERS));
	res = enum_adapter(&adapters);
    assert(res == STATUS_SUCCESS && adapters.count > 0);

	info.adapter = adapters.adapters[0].handle;
    info.escape = escape;

    State::initializeState();

    DbgPrint(TRACE_LEVEL_INFO, ("[?] ICD Initialized.\n"));
    return info;
}

static NTSTATUS sendKernel(D3DKMT_ESCAPE *escape_info, PFND3DKMT_ESCAPE escape)
{
    TRACE_IN();
    NTSTATUS res = STATUS_SUCCESS;

    if (Tests::test_enabled) {
        Tests::dumpCommandBuffer(escape_info->privateDriverData, escape_info->privateDriverDataSize);
        res = STATUS_SUCCESS;
    }
    else
        res = escape(escape_info);

    TRACE_OUT();
    return res;
}

VOID sendCommand(VOID *command, UINT32 size)
{
    TRACE_IN();

	D3DKMT_ESCAPE escape_info = { 0 };
    NTSTATUS res = STATUS_SUCCESS;
	device_info_t info;
    VOID *data = NULL;

    info = initialize_device();
    if (!command)
        return;

    data = new BYTE[size + sizeof(UINT32)];
    assert(data);
    *(UINT32*)data = DRIVER_CMD_TRANSFER;
    memcpy_s((UINT32*)data + 1, size, command, size);

	escape_info.hAdapter = info.adapter;
	escape_info.hDevice = NULL;
	escape_info.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape_info.flags.Value = 1;
	escape_info.hContext = NULL;
	escape_info.privateDriverData = data;
	escape_info.privateDriverDataSize = size + sizeof(UINT32);

    res = sendKernel(&escape_info, info.escape);

    delete[] data;

    if (res != STATUS_SUCCESS)
        DbgPrint(TRACE_LEVEL_ERROR, ("[!] %s: Escape returned with error 0x%x (%s)\n", __FUNCTION__, res, status2str(res)));
    assert(res == STATUS_SUCCESS);
    TRACE_OUT();
}

UINT64 allocate_object(UINT32 size)
{
    TRACE_IN();
    device_info_t info = initialize_device();

    gpu_allocate_object_t data = { 0 };
    data.driver_cmd = DRIVER_CMD_ALLOCATE;
    data.size = size;
    
	D3DKMT_ESCAPE escape_info = { 0 };
	escape_info.hAdapter = info.adapter;
	escape_info.hDevice = NULL;
	escape_info.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape_info.flags.Value = 1;
	escape_info.hContext = NULL;
	escape_info.privateDriverData = &data;
	escape_info.privateDriverDataSize = sizeof(data);
    
    sendKernel(&escape_info, info.escape);

    TRACE_OUT();
    return data.handle;
}

VOID update_object(UINT64 handle, VOID *ptr, UINT32 size)
{
    TRACE_IN();
    device_info_t info = initialize_device();

    gpu_update_object_t data = { 0 };
    data.driver_cmd = DRIVER_CMD_UPDATE;
    data.handle = handle;
    data.ptr = ptr;
    data.size = size;
    
	D3DKMT_ESCAPE escape_info = { 0 };
	escape_info.hAdapter = info.adapter;
	escape_info.hDevice = NULL;
	escape_info.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape_info.flags.Value = 1;
	escape_info.hContext = NULL;
	escape_info.privateDriverData = &data;
	escape_info.privateDriverDataSize = sizeof(data);
    
    sendKernel(&escape_info, info.escape);
    TRACE_OUT();
}

VOID delete_object(UINT64 handle)
{
    TRACE_IN();
    device_info_t info = initialize_device();

    gpu_delete_object_t data = { 0 };
    data.driver_cmd = DRIVER_CMD_UPDATE;
    data.handle = handle;
    
	D3DKMT_ESCAPE escape_info = { 0 };
	escape_info.hAdapter = info.adapter;
	escape_info.hDevice = NULL;
	escape_info.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape_info.flags.Value = 1;
	escape_info.hContext = NULL;
	escape_info.privateDriverData = &data;
	escape_info.privateDriverDataSize = sizeof(data);
    
    sendKernel(&escape_info, info.escape);
    TRACE_OUT();
}