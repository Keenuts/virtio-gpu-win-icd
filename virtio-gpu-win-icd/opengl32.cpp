#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windef.h>
#include <WinBase.h>
#include <winnt.h>

#include "opengl32.h"
#include "GLtypes.h"
#include "winTypes.h"

#define TRANSFER_FUNCTION "D3DKMTEnumAdapters"

//static
LONG (*getGDIFunction(LPCSTR procName))(_Inout_ const void*)
{
	static HMODULE lib = NULL;
	if (lib == NULL)
		lib = LoadLibrary("gdi32.dll");
	return (LONG(*)(_Inout_ const void*))GetProcAddress(lib, procName);
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
	void *command = new BYTE[sizeof(UINT32) + size];

	UINT32 *head = (UINT32*)command;
	*head = gnu_hash(name);

	if (size > 0)
		memcpy(head + 1, payload, size);
	//TODO: Implement this function on the kernel side
	//getGDIFunction(TRANSFER_FUNCTION)(command);
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