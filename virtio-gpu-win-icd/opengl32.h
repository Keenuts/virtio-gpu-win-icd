#pragma once

#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windef.h>
#include <WinBase.h>
#include <winnt.h>

#include "GLtypes.h"

#define EXPORT extern "C"

EXPORT void WINAPI glBegin(GLenum mode);

EXPORT void WINAPI glClear(GLbitfield mask);
EXPORT void WINAPI glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
EXPORT void WINAPI glClearDepth(GLdouble depth);
EXPORT void WINAPI glClearStencil(GLint stencil);

EXPORT void WINAPI glColor3f(GLfloat r, GLfloat g, GLfloat b);
EXPORT void WINAPI glEnd(void);
EXPORT void WINAPI glFlush(void);
EXPORT void WINAPI glVertex2i(GLint x, GLint y);
EXPORT void WINAPI glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
EXPORT HGLRC WINAPI wglCreateContext(HDC hdc);
EXPORT BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hglrc);
EXPORT BOOL WINAPI wglDeleteContext(HGLRC hglrc);
EXPORT BOOL WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);
EXPORT int WINAPI wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, PPIXELFORMATDESCRIPTOR ppfd);
EXPORT BOOL WINAPI wglSetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd);