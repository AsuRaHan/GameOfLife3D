#pragma once
#include <windows.h>
//#include <windowsx.h>
#include <cstdio> // For swprintf and swprintf_s
#include <gl/GL.h>
#include <GL/glu.h>

#ifndef GLFUNC_H
#define GLFUNC_H
// Определение идентификаторов, если они не определены
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif

#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif

#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif

#ifndef GL_STREAM_DRAW
#define GL_STREAM_DRAW 0x88E0
#endif

#ifndef GL_ARRAY_BUFFER_BINDING
#define GL_ARRAY_BUFFER_BINDING 0x8894
#endif

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif

// Определение если он не определен
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

// Определение типов для указателей на функции
typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (APIENTRY* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRY* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef void (APIENTRY* PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);

// Объявление указателей на функции
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;

// Функции для работы с OpenGL
void LoadOpenGLFunctions();
void CheckOpenGLError(const char* stmt, const char* fname, int line);

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
    stmt; \
    CheckOpenGLError(#stmt, __FILE__, __LINE__); \
} while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

#endif //GLFUNC_H