#include <cassert>
#include <intrin.h>

#include "debug.h"
#include "tests.h"
#include "virgl.h"
#include "virgl_command.h"
#include "state.h"
#include "win_types.h"

#define TRANSFER_FUNCTION "D3DKMTEscape"

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

typedef struct _DRIVER_DATA {
    HMODULE lib;
    PFND3DKMT_ESCAPE escape;
    PFND3DKMT_ENUMADAPTERS enum_adapters;
    D3DKMT_HANDLE adapter;
} DRIVER_DATA, *PDRIVER_DATA;

template<typename PFUNC>
PFUNC getGDIFunction(PDRIVER_DATA data, LPCSTR procName)
{
	return reinterpret_cast<PFUNC>(GetProcAddress(data->lib, procName));
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

static PDRIVER_DATA initialize_device() {

    static PDRIVER_DATA driver_data = NULL;
    static BOOL initialized = FALSE;
	NTSTATUS res;
	D3DKMT_ENUMADAPTERS adapters;

    if (initialized)
        return driver_data;
    initialized = TRUE;

#if _DEBUG
#else
    if (!Tests::test_enabled) {
        FILE *com_fd;
        assert(freopen_s(&com_fd, "COM2:", "w", stdout) == 0);
    }
#endif

    DbgPrint(TRACE_LEVEL_ERROR, ("[?] Starting ICD Build on %s %s: .\n", __DATE__, __TIME__));

    driver_data = new DRIVER_DATA();
    assert(driver_data);
    driver_data->lib = LoadLibrary(L"gdi32.dll");
    assert(driver_data->lib);

	driver_data->enum_adapters = getGDIFunction<PFND3DKMT_ENUMADAPTERS>(driver_data, "D3DKMTEnumAdapters");
	driver_data->escape = getGDIFunction<PFND3DKMT_ESCAPE>(driver_data, "D3DKMTEscape");
    assert(driver_data->enum_adapters);
    assert(driver_data->escape);

	memset(&adapters, 0, sizeof(D3DKMT_ENUMADAPTERS));
	res = driver_data->enum_adapters(&adapters);
    assert(res == STATUS_SUCCESS && adapters.count > 0);

	driver_data->adapter = adapters.adapters[0].handle;

    State::initializeState();

    DbgPrint(TRACE_LEVEL_INFO, ("[?] ICD Initialized.\n"));
    DbgPrint(TRACE_LEVEL_ERROR, ("Escape function: %p PDRIVER_DATA = %p\n", driver_data->escape, driver_data));
    return driver_data;
}

static NTSTATUS sendKernel(D3DKMT_ESCAPE *escape_info, PFND3DKMT_ESCAPE escape)
{
    TRACE_IN();
    NTSTATUS res = STATUS_SUCCESS;

#if _DEBUG
    if (Tests::test_enabled) {
        Tests::dumpCommandBuffer(escape_info->privateDriverData, escape_info->privateDriverDataSize);
        res = STATUS_SUCCESS;
    }
    UNREFERENCED_PARAMETER(escape);
#else
    if (Tests::test_enabled) {
        Tests::dumpCommandBuffer(escape_info->privateDriverData, escape_info->privateDriverDataSize);
        res = STATUS_SUCCESS;
    }
    else {
        res = escape(escape_info);
        if (res != STATUS_SUCCESS)
            DbgPrint(TRACE_LEVEL_ERROR, ("Escape returned: %s(0x%x)\n", State::errorToStr(res), res));
        assert(res == STATUS_SUCCESS);
    }
#endif

    TRACE_OUT();
    return res;
}

VOID sendCommand(VOID *command, UINT32 size)
{
    TRACE_IN();

	D3DKMT_ESCAPE escape_info = { 0 };
    NTSTATUS res = STATUS_SUCCESS;
	PDRIVER_DATA info;
    VOID *data = NULL;

    info = initialize_device();
    if (!command)
        return;

    data = new BYTE[size + sizeof(UINT32)];
    assert(data);
    *(UINT32*)data = DRIVER_CMD_TRANSFER;
    memcpy((UINT32*)data + 1, command, size);

	escape_info.hAdapter = info->adapter;
	escape_info.hDevice = NULL;
	escape_info.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape_info.flags.HardwareAccess = 1;
	escape_info.hContext = NULL;
	escape_info.privateDriverData = data;
	escape_info.privateDriverDataSize = size + sizeof(UINT32);

    res = sendKernel(&escape_info, info->escape);

    delete[] data;

    if (res != STATUS_SUCCESS)
        DbgPrint(TRACE_LEVEL_ERROR, ("[!] %s: Escape returned with error 0x%x (%s)\n", __FUNCTION__, res, status2str(res)));
    assert(res == STATUS_SUCCESS);
    TRACE_OUT();
}

UINT64 allocate_object(UINT32 size)
{
    TRACE_IN();
    PDRIVER_DATA info = initialize_device();

    gpu_allocate_object_t data = { 0 };
    data.driver_cmd = DRIVER_CMD_ALLOCATE;
    data.size = size;
    
	D3DKMT_ESCAPE escape_info = { 0 };
	escape_info.hAdapter = info->adapter;
	escape_info.hDevice = NULL;
	escape_info.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape_info.flags.HardwareAccess = 1;
	escape_info.hContext = NULL;
	escape_info.privateDriverData = &data;
	escape_info.privateDriverDataSize = sizeof(data);
    
    sendKernel(&escape_info, info->escape);

    TRACE_OUT();
    return data.handle;
}

VOID update_object(UINT64 handle, VOID *ptr, UINT32 size)
{
    TRACE_IN();
    PDRIVER_DATA info = initialize_device();

    gpu_update_object_t data = { 0 };
    data.driver_cmd = DRIVER_CMD_UPDATE;
    data.handle = handle;
    data.ptr = ptr;
    data.size = size;
    
	D3DKMT_ESCAPE escape_info = { 0 };
	escape_info.hAdapter = info->adapter;
	escape_info.hDevice = NULL;
	escape_info.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape_info.flags.HardwareAccess = 1;
	escape_info.hContext = NULL;
	escape_info.privateDriverData = &data;
	escape_info.privateDriverDataSize = sizeof(data);
    
    sendKernel(&escape_info, info->escape);
    TRACE_OUT();
}

VOID delete_object(UINT64 handle)
{
    TRACE_IN();
    PDRIVER_DATA info = initialize_device();

    gpu_delete_object_t data = { 0 };
    data.driver_cmd = DRIVER_CMD_UPDATE;
    data.handle = handle;
    
	D3DKMT_ESCAPE escape_info = { 0 };
	escape_info.hAdapter = info->adapter;
	escape_info.hDevice = NULL;
	escape_info.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape_info.flags.HardwareAccess = 1;
	escape_info.hContext = NULL;
	escape_info.privateDriverData = &data;
	escape_info.privateDriverDataSize = sizeof(data);
    
    sendKernel(&escape_info, info->escape);
    TRACE_OUT();
}