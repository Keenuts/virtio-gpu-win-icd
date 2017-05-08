#include <stdio.h>
#include <windef.h>
#include <WinBase.h>
#include <winnt.h>

#include "GLtypes.h"
#include "winTypes.h"

void WINAPI glBegin(GLenum mode )
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);
	
	UNREFERENCED_PARAMETER(mode);
}

void WINAPI glClear( GLbitfield mask )
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);

	UNREFERENCED_PARAMETER(mask);
}

void WINAPI glColor3f( GLfloat r, GLfloat b, GLfloat c )
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);
	
	UNREFERENCED_PARAMETER(r);
	UNREFERENCED_PARAMETER(b);
	UNREFERENCED_PARAMETER(c);
}

void WINAPI glEnd(void)
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);
}

void WINAPI glFlush(void)
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);
}

void WINAPI glVertex2i( GLint x, GLint y )
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);

	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void WINAPI glViewport( GLint x, GLint y, unsigned int width, unsigned int height )
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);

	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(width);
	UNREFERENCED_PARAMETER(height);
}

HGLRC WINAPI wglCreateContext(HDC hdc)
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);

	UNREFERENCED_PARAMETER(hdc);
	return NULL;
}

BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);

	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(hglrc);
	return TRUE;
}

BOOL WINAPI wglDeleteContext(HGLRC hglrc)
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);

	UNREFERENCED_PARAMETER(hglrc);
	return TRUE;
}

BOOL WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd)
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);
	
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(ppfd);

	PIXELFORMATDESCRIPTOR descriptor;
	descriptor.nSize = 0x28;
	descriptor.nVersion = 0x1;
	descriptor.dwFlags = 0x24;
	descriptor.cColorBits = 0x20;

	memcpy((void*)ppfd, &descriptor, sizeof(descriptor));
	return TRUE;
}

int WINAPI wglDescribePixelFormat(HDC hdc, int iPixelFormat)
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);

	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(iPixelFormat);
	return 16;
}

BOOL WINAPI wglSetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd)
{
	DebugBreak();
	printf( "Calling %s\n", __FUNCTION__);

	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(iPixelFormat);
	UNREFERENCED_PARAMETER(ppfd);

	return TRUE;
}