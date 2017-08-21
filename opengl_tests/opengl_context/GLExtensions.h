#pragma once

#include "stdafx.h"

#include <assert.h>
#include <windows.h>			/* must include this before GL/gl.h */
#include <gl/GL.h>
#include <gl/GLU.h>

typedef UINT64 GLsizeiptr;
typedef UINT64 GLintptr;
typedef CHAR GLchar;

#define GL_VERTEX_SHADER 0
#define GL_FRAGMENT_SHADER 1
#define GL_ARRAY_BUFFER 0
#define GL_STATIC_DRAW 0

/* NOTE: Comments are based on the Virtio ICD POC
 * These functions are not designed to work as the original
 * They are here to give the illusion of a full ICD.
 * All shader related behaviour is in fact completely fixed.
 * However, vertex related commands are expected to behave a bit
 * like the real one (you can change vtx count and values, but not
 * the layout).
 */

typedef VOID(*PFN_GLBINDATTRIBLOCATION)(GLuint program, GLuint index, const GLchar *name);
typedef VOID(*PFN_GLGENBUFFERS)(GLsizei n, GLuint *buffers);
typedef VOID(*PFN_GLBINDBUFFER)(GLenum target, GLuint buffer);
typedef VOID(*PFN_GLBUFFERDATA)(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef VOID(*PFN_GLBUFFERSUBDATA)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
typedef VOID(*PFN_GLVERTEXATTRIBPOINTER)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);
typedef VOID(*PFN_GLENABLEVERTEXATTRIBARRAY)(GLuint index);

template<typename PFUNC>
static PFUNC getOpenGLFunction(HMODULE lib, LPCSTR procName)
{
	return reinterpret_cast<PFUNC>(GetProcAddress(lib, procName));
}


static void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    HMODULE h = LoadLibrary(L"opengl32.dll");
    assert(h);
    
    PFN_GLBINDATTRIBLOCATION func = getOpenGLFunction<PFN_GLBINDATTRIBLOCATION>(h, "glBindAttribLocation");
    assert(func);

    func(program, index, name);
}

static void glGenBuffers(GLsizei n, GLuint *buffers)
{
    HMODULE h = LoadLibrary(L"opengl32.dll");
    assert(h);
    PFN_GLGENBUFFERS func = getOpenGLFunction<PFN_GLGENBUFFERS>(h, "glGenBuffers");
    assert(func);
    func(n, buffers);
}

static void glBindBuffer(GLenum target, GLuint buffer)
{
    HMODULE h = LoadLibrary(L"opengl32.dll");
    assert(h);
    PFN_GLBINDBUFFER func = getOpenGLFunction<PFN_GLBINDBUFFER>(h, "glBindBuffer");
    assert(func);
    func(target, buffer);
}

static void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    HMODULE h = LoadLibrary(L"opengl32.dll");
    assert(h);
    PFN_GLBUFFERDATA func = getOpenGLFunction<PFN_GLBUFFERDATA>(h, "glBufferData");
    assert(func);
    func(target, size, data, usage);
}

static void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
    HMODULE h = LoadLibrary(L"opengl32.dll");
    assert(h);
    PFN_GLBUFFERSUBDATA func = getOpenGLFunction<PFN_GLBUFFERSUBDATA>(h, "glBufferSubData");
    assert(func);
    func(target, offset, size, data);
}

static void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
{
    HMODULE h = LoadLibrary(L"opengl32.dll");
    assert(h);
    PFN_GLVERTEXATTRIBPOINTER func = getOpenGLFunction<PFN_GLVERTEXATTRIBPOINTER>(h, "glVertexAttribPointer");
    assert(func);
    func(index, size, type, normalized, stride, pointer);
}

static void glEnableVertexAttribArray(GLuint index)
{
    HMODULE h = LoadLibrary(L"opengl32.dll");
    assert(h);
    PFN_GLENABLEVERTEXATTRIBARRAY func = getOpenGLFunction<PFN_GLENABLEVERTEXATTRIBARRAY>(h, "glEnableVertexAttribArray");
    assert(func);
    func(index);
}
