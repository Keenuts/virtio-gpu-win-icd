#include <stdio.h>
#include <windef.h>
#include <WinBase.h>
#include <winnt.h>

#include "GLtypes.h"
#include "winTypes.h"

HMODULE loadGDI()
{
	return NULL;
}

void WINAPI glBegin(GLenum mode )
{
	UNREFERENCED_PARAMETER(mode);
}

void WINAPI glClear( GLbitfield mask )
{
	UNREFERENCED_PARAMETER(mask);
}

void WINAPI glColor3f( GLfloat r, GLfloat b, GLfloat c )
{
	UNREFERENCED_PARAMETER(r);
	UNREFERENCED_PARAMETER(b);
	UNREFERENCED_PARAMETER(c);
}

void WINAPI glEnd(void)
{
}

void WINAPI glFlush(void)
{
}

void WINAPI glVertex2i( GLint x, GLint y )
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void WINAPI glViewport( GLint x, GLint y, unsigned int width, unsigned int height )
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(width);
	UNREFERENCED_PARAMETER(height);
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
	//DebugBreak();
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
	//DebugBreak();
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