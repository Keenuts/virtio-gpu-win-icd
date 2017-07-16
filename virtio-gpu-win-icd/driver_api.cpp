#include <cassert>

#include "debug.h"
#include "virgl_command.h"
#include "win_types.h"

#define TRANSFER_FUNCTION "D3DKMTEscape"

struct device_info_t {
	D3DKMT_HANDLE adapter;
    PFND3DKMT_ESCAPE escape;
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

static void initialize_device(device_info_t *info) {
	NTSTATUS res;
    FILE *com_fd;
	D3DKMT_ENUMADAPTERS adapters;
	PFND3DKMT_ENUMADAPTERS enum_adapter;
    PFND3DKMT_ESCAPE escape;

    assert(freopen_s(&com_fd, "COM2:", "w", stdout) == 0);
    DbgPrint(TRACE_LEVEL_INFO, ("[?] Starting ICD Build on %s %s: .\n", __DATE__, __TIME__));

	enum_adapter = getGDIFunction<PFND3DKMT_ENUMADAPTERS>("D3DKMTEnumAdapters");
	escape = getGDIFunction<PFND3DKMT_ESCAPE>("D3DKMTEscape");
    assert(enum_adapter);
    assert(escape);

	memset(&adapters, 0, sizeof(D3DKMT_ENUMADAPTERS));
	res = enum_adapter(&adapters);
    assert(res == STATUS_SUCCESS && adapters.count > 0);

	info->adapter = adapters.adapters[0].handle;
    info->escape = escape;

    DbgPrint(TRACE_LEVEL_INFO, ("[?] ICD Initialized.\n"));
}

void sendCommand(void *command, UINT32 size)
{
    DbgPrint(TRACE_LEVEL_INFO, ("--> %s.\n", __FUNCTION__));

	static bool initialized = false;
	static device_info_t info;

	D3DKMT_ESCAPE escape_info = { 0 };
    NTSTATUS res = STATUS_SUCCESS;

	if (!initialized) {
		initialize_device(&info);
		initialized = true;
        VirGL::printHost("[?] Initializing OpenGL ICD\n");
	}

    if (!command)
        return;

	escape_info.hAdapter = info.adapter;
	escape_info.hDevice = NULL;
	escape_info.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape_info.flags.Value = 1;
	escape_info.hContext = NULL;
	escape_info.privateDriverData = command;
	escape_info.privateDriverDataSize = size;

    res = info.escape(&escape_info);
    if (res != STATUS_SUCCESS)
        DbgPrint(TRACE_LEVEL_ERROR, ("[!] %s: Escape returned with error 0x%x (%s)\n", __FUNCTION__, res, status2str(res)));
    assert(res == STATUS_SUCCESS);

    DbgPrint(TRACE_LEVEL_INFO, ("<-- %s.\n", __FUNCTION__));
}
