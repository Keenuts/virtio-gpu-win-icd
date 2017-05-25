#include "helper.h"
#include "opengl32.h"

#define TRANSFER_FUNCTION "D3DKMTEscape"

struct device_info_t {
	VOID* adapter;
	VOID* device;
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
	D3DKMT_CREATEDEVICE device_info;
	PFND3DKMT_CREATEDEVICE create_device = getGDIFunction<PFND3DKMT_CREATEDEVICE>("D3DKMTCreateDevice");
	PFND3DKMT_ENUMADAPTERS enum_adapter = getGDIFunction <PFND3DKMT_ENUMADAPTERS> ("D3DKMTEnumAdapters");
	_ASSERT(create_device);
	_ASSERT(enum_adapter);
	
	D3DKMT_ENUMADAPTERS adapters;
	memset(&adapters, 0, sizeof(D3DKMT_ENUMADAPTERS));
	res = enum_adapter(&adapters);
	_ASSERT(res == STATUS_SUCCESS);
	info->adapter = adapters.adapters[0].handle;

	memset(&device_info, 0, sizeof(D3DKMT_CREATEDEVICE));
	device_info.adapter = info->adapter;
	res = create_device(&device_info);
	_ASSERT(res == STATUS_SUCCESS);
	info->device = device_info.device;
}

// From Oracle blog GNU HASH for elf symbols
static UINT32 gnu_hash(const char* s) {
	UINT32 h = 5381;
	for (UCHAR c = *s; c != 0; c = *s++)
		h = h * 33 + c;
	return h;
}

static void sendCommand(const char *name, void *payload = NULL, UINT32 size = 0)
{
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

	//TODO: Implement this function on the kernel side
	D3DKMT_ESCAPE escape = { 0 };
	escape.adapter = info.adapter;
	escape.device = info.device;
	escape.type = D3DKMT_ESCAPE_DRIVERPRIVATE;
	escape.flags = 0;
	escape.context = NULL;

	escape.privateDriverData = command;
	escape.privateDriverDataSize = commandSize;

	PFND3DKMT_ESCAPE func = getGDIFunction<PFND3DKMT_ESCAPE>(TRANSFER_FUNCTION);
	func(&escape);

	delete command;
}

void WINAPI glBegin(GLenum mode )
{
	sendCommand(__FUNCTION__, &mode, sizeof(mode));
}

void WINAPI glClear( GLbitfield mask )
{
	sendCommand(__FUNCTION__, &mask, sizeof(mask));
}

void WINAPI glColor3f( GLfloat r, GLfloat g, GLfloat b)
{
	GLfloat data[] = { r, g, b };
	sendCommand(__FUNCTION__, data, sizeof(data));
}

void WINAPI glEnd(void)
{
	sendCommand(__FUNCTION__);
}

void WINAPI glFlush(void)
{
	sendCommand(__FUNCTION__);
}

void WINAPI glVertex2i( GLint x, GLint y )
{
	GLint data[] = { x, y };
	sendCommand(__FUNCTION__, data, sizeof(data));
}

struct viewport_data_t {
	GLint x, y;
	GLsizei width, height;
};

void WINAPI glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	viewport_data_t data = { x, y, width, height };
	sendCommand(__FUNCTION__, &data, sizeof(data));
}

HGLRC WINAPI wglCreateContext(HDC hdc)
{
	UNREFERENCED_PARAMETER(hdc);
	return NULL;
}

BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(hglrc);
	return TRUE;
}

BOOL WINAPI wglDeleteContext(HGLRC hglrc)
{
	UNREFERENCED_PARAMETER(hglrc);
	return TRUE;
}

BOOL WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd)
{
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
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(iPixelFormat);
	UNREFERENCED_PARAMETER(nBytes);
	UNREFERENCED_PARAMETER(ppfd);
	return 16;
}

BOOL WINAPI wglSetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd)
{
	//DebugBreak();
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(iPixelFormat);
	UNREFERENCED_PARAMETER(ppfd);

	return TRUE;
}