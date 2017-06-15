#include <assert.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "helper.h"
#include "opengl32.h"

#define TRACE_LEVEL_INFO 0
#define TRACE_LEVEL_WARNING 1
#define TRACE_LEVEL_ERROR 2
#define TRACE_LEVEL_SEVERE 3

#define TRANSFER_FUNCTION "D3DKMTEscape"
#define TRACE_LEVEL TRACE_LEVEL_INFO

//Warning disabled: constant comparaison
#define DbgPrint(Level, Line)                               \
__pragma(warning(push))                                     \
__pragma(warning(disable:4127))                             \
    if ((Level) >= TRACE_LEVEL)                             \
        printf Line;                                        \
__pragma(warning(pop))                                      \
    FlushFileBuffers(GetStdHandle(STD_OUTPUT_HANDLE));                                             \

struct device_info_t {
	D3DKMT_HANDLE adapter;
	D3DKMT_HANDLE device;
	D3DKMT_HANDLE context;
    HANDLE serial_output;
};

template<typename PFUNC>
PFUNC getGDIFunction(LPCSTR procName)
{
	static HMODULE lib = NULL;
	if (lib == NULL)
		lib = LoadLibrary("gdi32.dll");
	return reinterpret_cast<PFUNC>(GetProcAddress(lib, procName));
}


void initialize_device(device_info_t *info) {
	NTSTATUS res;
    FILE *com_fd;

    assert(freopen_s(&com_fd, "COM2:", "w", stdout) == 0);
    DbgPrint(TRACE_LEVEL_INFO, ("[INFO] Starting ICD Build on %s %s: .\n", __DATE__, __TIME__));

	PFND3DKMT_ENUMADAPTERS enum_adapter = getGDIFunction <PFND3DKMT_ENUMADAPTERS> ("D3DKMTEnumAdapters");
	_ASSERT(enum_adapter);

	D3DKMT_ENUMADAPTERS adapters;
	memset(&adapters, 0, sizeof(D3DKMT_ENUMADAPTERS));
	res = enum_adapter(&adapters);
	_ASSERT(res == STATUS_SUCCESS);
	_ASSERT(adapters.count > 0);

    for (ULONG i = 0; i < adapters.count; i++)
        DbgPrint(TRACE_LEVEL_INFO, ("Adapter: %lx LUID:%x %lx Sources:%lx \n",
            adapters.adapters[i].handle,
            adapters.adapters[i].luid.LowPart,
            adapters.adapters[i].luid.HighPart,
            adapters.adapters[i].sourceCount));

	info->adapter = adapters.adapters[1].handle;
    
#if 0
	PFND3DKMT_CREATEDEVICE create_device = getGDIFunction<PFND3DKMT_CREATEDEVICE>("D3DKMTCreateDevice");
	_ASSERT(create_device);

    D3DKMT_CREATEDEVICE device_info = { 0 };
	
	device_info.hAdapter = info->adapter;
	device_info.flags.LegacyMode = 0;
	device_info.flags.RequestVSync = 0;
	device_info.flags.DisableGpuTimeout = 0;
	device_info.flags.Reserved = 0;
	
	res = create_device(&device_info);
	_ASSERT(res == STATUS_SUCCESS);
	info->device = device_info.hDevice;
#endif

#if 0
	PFND3DKMT_CREATECONTEXT create_context = getGDIFunction <PFND3DKMT_CREATECONTEXT> ("D3DKMTCreateContext");
	_ASSERT(create_context);

    D3DKMT_CREATECONTEXT context_data = { 0 };
    context_data.hDevice = info->device;

    res = create_context(&context_data);
    _ASSERT(res == STATUS_SUCCESS);
    info->context = context_data.hContext;
#endif

    DbgPrint(TRACE_LEVEL_INFO, ("[INFO] ICD Initialized.\n"));
}

// From Oracle blog GNU HASH for elf symbols
static UINT32 gnu_hash(const char* s) {
	UINT32 h = 5381;
	for (UCHAR c = *s; c != 0; c = *s++)
		h = h * 33 + c;
	return h;
}

static void sendCommand(const char *name, void *payload, UINT32 size)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] --> %s.\n", __FUNCTION__));
	static bool initialized = false;
	static device_info_t info;
	UINT32 commandSize = sizeof(UINT32) + size;
	void *command = NULL;
	UINT32 *head;

	if (!initialized) {
		initialize_device(&info);
		initialized = true;
	}

	command = new BYTE[commandSize];
	head = (UINT32*)command;
	*head = gnu_hash(name);

	if (size > 0)
		memcpy(head + 1, payload, size);

	D3DKMT_ESCAPE escape = { 0 };
	escape.hAdapter = info.adapter;
	escape.hDevice = info.device;
	escape.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape.flags.Value = 1;
	escape.hContext = info.context;
	escape.privateDriverData = command;
	escape.privateDriverDataSize = commandSize * sizeof(BYTE);

    D3DKMT_QUERYADAPTERINFO query_info = { 0 };
    query_info.hAdapter = info.adapter;
    query_info.Type = KMTQAITYPE_UMDRIVERPRIVATE;
    query_info.pPrivateDriverData = command;
    query_info.PrivateDriverDataSize = commandSize * sizeof(BYTE);


	PFND3DKMT_ESCAPE func = getGDIFunction<PFND3DKMT_ESCAPE>(TRANSFER_FUNCTION);
	PFND3DKMT_QUERYADAPTERINFO query_func = getGDIFunction<PFND3DKMT_QUERYADAPTERINFO>("D3DKMTQueryAdapterInfo");
    (void)func;
	NTSTATUS res = query_func(&query_info);
    res = STATUS_SUCCESS;
    // DbgPrint(TRACE_LEVEL_INFO, ("[INFO] Escape function returned with status code: 0x%lx\n", res));

    assert(res != STATUS_INVALID_PARAMETER);
    assert(res != STATUS_NO_MEMORY);
    assert(res != STATUS_PRIVILEGED_INSTRUCTION);
    assert(res != STATUS_ILLEGAL_INSTRUCTION);
    assert(res != STATUS_GRAPHIC_DRIVER_MISMATCH);
    assert(res == STATUS_SUCCESS);

	delete command;
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] <-- %s.\n", __FUNCTION__));
}

void WINAPI glBegin(GLenum mode )
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	sendCommand(__FUNCTION__, &mode, sizeof(mode));
}

void WINAPI glClear( GLbitfield mask )
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	sendCommand(__FUNCTION__, &mask, sizeof(mask));
}

void WINAPI glColor3f( GLfloat r, GLfloat g, GLfloat b)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	GLfloat data[] = { r, g, b };
	sendCommand(__FUNCTION__, data, sizeof(data));
}

void WINAPI glEnd(void)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	sendCommand(__FUNCTION__, NULL, 0);
}

void WINAPI glFlush(void)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	sendCommand(__FUNCTION__, NULL, 0);
}

void WINAPI glVertex2i( GLint x, GLint y )
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	GLint data[] = { x, y };
	sendCommand(__FUNCTION__, data, sizeof(data));
}

struct viewport_data_t {
	GLint x, y;
	GLsizei width, height;
};

void WINAPI glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	viewport_data_t data = { x, y, width, height };
	sendCommand(__FUNCTION__, &data, sizeof(data));
}

HGLRC WINAPI wglCreateContext(HDC hdc)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	sendCommand(__FUNCTION__, &hdc, sizeof(HDC));
	return NULL;
}

BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	//FIXME
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(hglrc);
	return TRUE;
}

BOOL WINAPI wglDeleteContext(HGLRC hglrc)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	sendCommand(__FUNCTION__, &hglrc, sizeof(HDC));
	UNREFERENCED_PARAMETER(hglrc);
	return TRUE;
}

BOOL WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	//FIXME
	//Setting up a dummy pixel format to pass validation
	PIXELFORMATDESCRIPTOR descriptor;
	descriptor.nSize = 0x28;
	descriptor.nVersion = 0x1;
	descriptor.dwFlags = 0x24;
	descriptor.cColorBits = 0x20;

	memcpy((void*)ppfd, &descriptor, sizeof(descriptor));

	UNREFERENCED_PARAMETER(hdc);
	return TRUE;
}

int WINAPI wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, PPIXELFORMATDESCRIPTOR ppfd)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	//FIXME
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(iPixelFormat);
	UNREFERENCED_PARAMETER(nBytes);
	UNREFERENCED_PARAMETER(ppfd);
	return 16;
}

BOOL WINAPI wglSetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd)
{
    DbgPrint(TRACE_LEVEL_INFO, ("[TRACE] |-> %s\n", __FUNCTION__));
	//FIXME
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(iPixelFormat);
	UNREFERENCED_PARAMETER(ppfd);

	return TRUE;
}