#include <stdio.h>
#include "GLtypes.h"

#include <windef.h>
#include <winnt.h>

void __cdecl glBegin(GLenum mode )
{
	UNREFERENCED_PARAMETER(mode);

	printf("Calling %s\n", __FUNCTION__);
}

void __cdecl glClear( GLbitfield mask )
{
	UNREFERENCED_PARAMETER(mask);

	printf("Calling %s\n", __FUNCTION__);
}

void __cdecl glColor3f( GLfloat r, GLfloat b, GLfloat c )
{
	UNREFERENCED_PARAMETER(r);
	UNREFERENCED_PARAMETER(b);
	UNREFERENCED_PARAMETER(c);

	printf("Calling %s\n", __FUNCTION__);
}

void __cdecl glEnd(void)
{
	printf("Calling %s\n", __FUNCTION__);
}

void __cdecl glFlush(void)
{
	printf("Calling %s\n", __FUNCTION__);
}

void __cdecl glVertex2i( GLint x, GLint y )
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);

	printf("Calling %s\n", __FUNCTION__);
}

void __cdecl glViewport( GLint x, GLint y, unsigned int width, unsigned int height )
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(width);
	UNREFERENCED_PARAMETER(height);

	printf("Calling %s\n", __FUNCTION__);
}

HGLRC __cdecl wglCreateContext(HDC hdc)
{
	UNREFERENCED_PARAMETER(hdc);
	printf("Calling %s\n", __FUNCTION__);
	int *ptr = NULL;
	int value = *ptr;
	(void)value;
	return NULL;
}

BOOL __cdecl wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(hglrc);
	printf("Calling %s\n", __FUNCTION__);
	return TRUE;
}

BOOL __cdecl wglDeleteContext(HGLRC hglrc)
{
	UNREFERENCED_PARAMETER(hglrc);
	printf("Calling %s\n", __FUNCTION__);
	return TRUE;
}