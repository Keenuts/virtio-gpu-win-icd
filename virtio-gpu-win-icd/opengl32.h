#pragma once

#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windef.h>
#include <WinBase.h>
#include <winnt.h>

#include "GLtypes.h"

typedef UINT64 GLsizeiptr;
typedef UINT64 GLintptr;
typedef CHAR GLchar;

#define GL_VERTEX_SHADER 0
#define GL_FRAGMENT_SHADER 1
#define GL_ARRAY_BUFFER 0
#define GL_STATIC_DRAW 0

#define EXPORT extern "C"

EXPORT void WINAPI glBegin(GLenum mode);

EXPORT void WINAPI glClear(GLbitfield mask);
EXPORT void WINAPI glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
EXPORT void WINAPI glClearDepth(GLdouble depth);
EXPORT void WINAPI glClearStencil(GLint stencil);

EXPORT void WINAPI glColor3f(GLfloat r, GLfloat g, GLfloat b);
EXPORT void WINAPI glDrawArrays(GLenum mode, GLint first, GLsizei count);
EXPORT void WINAPI glEnable(GLenum cap);
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

EXPORT void WINAPI glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
EXPORT void WINAPI glGenBuffers(GLsizei n, GLuint *buffers);
EXPORT void WINAPI glBindBuffer(GLenum target, GLuint buffer);
EXPORT void WINAPI glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
EXPORT void WINAPI glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
EXPORT void WINAPI glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);
EXPORT void WINAPI glEnableVertexAttribArray(GLuint index);
