#include <assert.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "debug.h"
#include "driver_api.h"
#include "helper.h"
#include "opengl32.h"
#include "virgl_command.h"
#include "win_types.h"


void WINAPI glBegin(GLenum mode )
{
    TRACE_IN();
    UNREFERENCED_PARAMETER(mode);
    TRACE_OUT();
}

void WINAPI glClear( GLbitfield mask )
{
    TRACE_IN();
    UNREFERENCED_PARAMETER(mask);
    TRACE_OUT();
}

void WINAPI glColor3f( GLfloat r, GLfloat g, GLfloat b)
{
    TRACE_IN();
	GLfloat data[] = { r, g, b };
    UNREFERENCED_PARAMETER(data);
    TRACE_OUT();
}

void WINAPI glEnd(void)
{
    TRACE_IN();
    TRACE_OUT();
}

void WINAPI glFlush(void)
{
    TRACE_IN();
    TRACE_OUT();
}

void WINAPI glVertex2i( GLint x, GLint y )
{
    TRACE_IN();
    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);
    TRACE_OUT();
}

struct viewport_data_t {
	GLint x, y;
	GLsizei width, height;
};

void WINAPI glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    TRACE_IN();
	viewport_data_t data = { x, y, width, height };
    UNREFERENCED_PARAMETER(data);
    TRACE_OUT();
}

#define DEFAULT_VGL_CTX 2
HGLRC WINAPI wglCreateContext(HDC hdc)
{
    static BOOL initialized = false;
    TRACE_IN();

    if (!initialized)
        sendCommand(NULL, 0);

    UNREFERENCED_PARAMETER(hdc);

    VirGL::createContext(DEFAULT_VGL_CTX);

    VirGL::VirglCommandBuffer cmd(DEFAULT_VGL_CTX);
    cmd.createSubContext(1);
    cmd.submitCommandBuffer();

    VirGL::deleteContext(DEFAULT_VGL_CTX);

    STOP();
    TRACE_OUT();
	return NULL;
}

BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
    TRACE_IN();
	//FIXME
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(hglrc);


    TRACE_OUT();
	return TRUE;
}

BOOL WINAPI wglDeleteContext(HGLRC hglrc)
{
    TRACE_IN();
    UNREFERENCED_PARAMETER(hglrc);

    TRACE_OUT();
	return TRUE;
}

BOOL WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd)
{
    TRACE_IN();
	//FIXME
	//Setting up a dummy pixel format to pass validation
	PIXELFORMATDESCRIPTOR descriptor;
	descriptor.nSize = 0x28;
	descriptor.nVersion = 0x1;
	descriptor.dwFlags = 0x24;
	descriptor.cColorBits = 0x20;

	memcpy((void*)ppfd, &descriptor, sizeof(descriptor));

	UNREFERENCED_PARAMETER(hdc);

    TRACE_OUT();
	return TRUE;
}

int WINAPI wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, PPIXELFORMATDESCRIPTOR ppfd)
{
    TRACE_IN();
	//FIXME
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(iPixelFormat);
	UNREFERENCED_PARAMETER(nBytes);
	UNREFERENCED_PARAMETER(ppfd);

    TRACE_OUT();
	return 16;
}

BOOL WINAPI wglSetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd)
{
    TRACE_IN();
	//FIXME
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(iPixelFormat);
	UNREFERENCED_PARAMETER(ppfd);

    TRACE_OUT();
	return TRUE;
}