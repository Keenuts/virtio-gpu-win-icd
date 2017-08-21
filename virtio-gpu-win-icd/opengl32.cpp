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
#include "state.h"
#include "win_types.h"

#define CallStateTracker(Function, ...)                                                                                                         \
    do {                                                                                                                                        \
        INT res = Function(__VA_ARGS__);                                                                                                        \
        if (res)                                                                                                                                \
            DbgPrint(TRACE_LEVEL_WARNING, ("[!] %s: state-tracker returned the error %d(%s)\n", __FUNCTION__, res, State::errorToStr(res)));    \
    } while (0)

void WINAPI glBegin(GLenum mode )
{
    TRACE_IN();

    CallStateTracker(State::begin);

    UNREFERENCED_PARAMETER(mode);
    TRACE_OUT();
}

void WINAPI glClear( GLbitfield mask )
{
    TRACE_IN();
    
    CallStateTracker(State::clear);

    UNREFERENCED_PARAMETER(mask);
    TRACE_OUT();
}

void WINAPI glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    TRACE_IN();

    CallStateTracker(State::clearColor, r, g, b, a);

    TRACE_OUT();
}

void WINAPI glClearDepth(GLdouble depth)
{
    TRACE_IN();

    CallStateTracker(State::clearDepth, depth);

    TRACE_OUT();
}

void WINAPI glClearStencil(GLint stencil)
{
    TRACE_IN();

    CallStateTracker(State::clearStencil , stencil);

    TRACE_OUT();
}

void WINAPI glColor3f( GLfloat r, GLfloat g, GLfloat b)
{
    TRACE_IN();

    State::push_color((float)r);
    State::push_vertex((float)g);
    State::push_vertex((float)b);

    TRACE_OUT();
}

void WINAPI glVertex2i( GLint x, GLint y )
{
    TRACE_IN();

    State::push_vertex((float)x);
    State::push_vertex((float)y);
    State::push_vertex((float)0);

    TRACE_OUT();
}

void WINAPI glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    TRACE_IN();

    UNREFERENCED_PARAMETER(mode);
    UNREFERENCED_PARAMETER(first);
    UNREFERENCED_PARAMETER(count);

    TRACE_OUT();
}

void WINAPI glEnable(GLenum cap)
{
    TRACE_IN();

    UNREFERENCED_PARAMETER(cap);

    TRACE_OUT();
}

void WINAPI glEnd(void)
{
    TRACE_IN();

    CallStateTracker(State::end);

    TRACE_OUT();
}

void WINAPI glFlush(void)
{
    TRACE_IN();
    State::flush();
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

#define APP_VGL_CTX 2
HGLRC WINAPI wglCreateContext(HDC hdc)
{
    static BOOL initialized = false;
    TRACE_IN();

    if (!initialized)
        sendCommand(NULL, 0);

    UNREFERENCED_PARAMETER(hdc);
    UINT32 ctx_id;

    VirGL::printHost("[STARTING OPENGL APP]\n");
    CallStateTracker(State::createContext, &ctx_id);

    TRACE_OUT();
	return (HGLRC)ctx_id;
}

BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
    TRACE_IN();

    UNREFERENCED_PARAMETER(hdc);

    UINT32 sub_ctx = (UINT32)(UINT64)hglrc;
    INT res = State::makeCurrent(sub_ctx);
    if (res)
        DbgPrint(TRACE_LEVEL_WARNING, ("[!] %s: state-tracker returned the error %d(%s)\n", __FUNCTION__, res, State::errorToStr(res)));

    TRACE_OUT();
	return res == STATUS_SUCCESS;
}

BOOL WINAPI wglDeleteContext(HGLRC hglrc)
{
    TRACE_IN();

    UINT32 sub_ctx = (UINT32)(UINT64)hglrc;
    INT res = State::deleteContext(sub_ctx);
    if (res)
        DbgPrint(TRACE_LEVEL_WARNING, ("[!] %s: state-tracker returned the error %d(%s)\n", __FUNCTION__, res, State::errorToStr(res)));

    VirGL::printHost("[ENDING OPENGL APP]\n");
    TRACE_OUT();
	return res == STATUS_SUCCESS;
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

void WINAPI glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    TRACE_IN();

    UNREFERENCED_PARAMETER(program);
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(name);

    TRACE_OUT();
}

void WINAPI glGenBuffers(GLsizei n, GLuint *buffers)
{
    TRACE_IN();

    UINT32 res = State::GenBuffers(n, buffers);
    assert(res == STATUS_SUCCESS);

    TRACE_OUT();
}

void WINAPI glBindBuffer(GLenum target, GLuint buffer)
{
    TRACE_IN();

    UINT32 res = State::BindBuffer(target, buffer);
    assert(res == STATUS_SUCCESS);

    TRACE_OUT();
}

void WINAPI glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    TRACE_IN();

    UINT32 res = State::BufferData(target, (UINT32)size, data, usage);
    assert(res == STATUS_SUCCESS);

    TRACE_OUT();
}

void WINAPI glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data)
{
    TRACE_IN();

    UINT32 res = State::BufferSubData(target, (UINT32)offset, (UINT32)size, data);
    assert(res == STATUS_SUCCESS);

    TRACE_OUT();
}

void WINAPI glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
{
    TRACE_IN();

    UINT32 res = State::VertexAttribPointer(index, size, type, normalized, stride, pointer);
    assert(res == STATUS_SUCCESS);

    TRACE_OUT();
}

void WINAPI glEnableVertexAttribArray(GLuint index)
{
    TRACE_IN();

    UINT32 res = State::EnableVertexAttribArray(index);
    assert(res == STATUS_SUCCESS);

    TRACE_OUT();
}