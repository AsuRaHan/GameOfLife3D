#pragma once
#ifndef GLFUNC_H
#define GLFUNC_H

#include <windows.h>
#include <gl/GL.h>
#include <GL/glu.h>

#include <iostream>
#include <cstdio> // For swprintf and swprintf_s

// Определение идентификаторов, если они не определены
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif
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
// Определение для инстансинга, если они не определены
#ifndef GL_DRAW_INDIRECT_BUFFER
#define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#endif
// Определения для шейдеров, если они не определены
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_GEOMETRY_SHADER
#define GL_GEOMETRY_SHADER 0x8DD9
#endif
#ifndef GL_TESS_CONTROL_SHADER
#define GL_TESS_CONTROL_SHADER 0x8E88
#endif
#ifndef GL_TESS_EVALUATION_SHADER
#define GL_TESS_EVALUATION_SHADER 0x8E87
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY 0x88B9
#endif
#ifndef GL_READ_ONLY
#define	GL_READ_ONLY 0x88B8
#endif
#ifndef GL_READ_WRITE
#define	GL_READ_WRITE 0x88BA
#endif
#ifndef GL_COMPUTE_SHADER
#define GL_COMPUTE_SHADER 0x91B9
#endif
#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#endif
#ifndef GL_DYNAMIC_COPY
#define GL_DYNAMIC_COPY 0x88EA
#endif
#ifndef GL_SHADER_STORAGE_BARRIER_BIT
#define GL_SHADER_STORAGE_BARRIER_BIT 0x00002000
#endif

#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif
#ifndef GL_NUM_EXTENSIONS
#define GL_NUM_EXTENSIONS 0x821D
#endif

#ifndef WGL_DRAW_TO_WINDOW_ARB
#define WGL_DRAW_TO_WINDOW_ARB                0x2001
#endif

#ifndef WGL_SUPPORT_OPENGL_ARB
#define WGL_SUPPORT_OPENGL_ARB                0x2010
#endif

#ifndef WGL_DOUBLE_BUFFER_ARB
#define WGL_DOUBLE_BUFFER_ARB                 0x2011
#endif

#ifndef WGL_PIXEL_TYPE_ARB
#define WGL_PIXEL_TYPE_ARB                    0x2013
#endif

#ifndef WGL_COLOR_BITS_ARB
#define WGL_COLOR_BITS_ARB                    0x2014
#endif

#ifndef WGL_DEPTH_BITS_ARB
#define WGL_DEPTH_BITS_ARB                    0x2022
#endif

#ifndef WGL_TYPE_RGBA_ARB
#define WGL_TYPE_RGBA_ARB                     0x202B
#endif

#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB
#define WGL_CONTEXT_MAJOR_VERSION_ARB         0x2091
#endif

#ifndef WGL_CONTEXT_MINOR_VERSION_ARB
#define WGL_CONTEXT_MINOR_VERSION_ARB         0x2092
#endif

#ifndef WGL_CONTEXT_FLAGS_ARB
#define WGL_CONTEXT_FLAGS_ARB                 0x2094
#endif

#ifndef WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#endif

#ifndef WGL_CONTEXT_PROFILE_MASK_ARB
#define WGL_CONTEXT_PROFILE_MASK_ARB          0x9126
#endif

#ifndef WGL_CONTEXT_CORE_PROFILE_BIT_ARB
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB      0x00000001
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif

#ifndef GL_DEBUG_SOURCE_API
#define GL_DEBUG_SOURCE_API 0x8246
#endif

#ifndef GL_DEBUG_SOURCE_WINDOW_SYSTEM
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#endif

#ifndef GL_DEBUG_SOURCE_SHADER_COMPILER
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#endif

#ifndef GL_DEBUG_SOURCE_THIRD_PARTY
#define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#endif

#ifndef GL_DEBUG_SOURCE_APPLICATION
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#endif

#ifndef GL_DEBUG_SOURCE_OTHER
#define GL_DEBUG_SOURCE_OTHER 0x824B
#endif

#ifndef GL_DEBUG_TYPE_ERROR
#define GL_DEBUG_TYPE_ERROR 0x824C
#endif

#ifndef GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#endif

#ifndef GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#endif

#ifndef GL_DEBUG_TYPE_PORTABILITY
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#endif

#ifndef GL_DEBUG_TYPE_PERFORMANCE
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#endif

#ifndef GL_DEBUG_TYPE_OTHER
#define GL_DEBUG_TYPE_OTHER 0x8251
#endif

#ifndef GL_MAX_DEBUG_MESSAGE_LENGTH
#define GL_MAX_DEBUG_MESSAGE_LENGTH 0x9143
#endif

#ifndef GL_MAX_DEBUG_LOGGED_MESSAGES
#define GL_MAX_DEBUG_LOGGED_MESSAGES 0x9144
#endif

#ifndef GL_DEBUG_LOGGED_MESSAGES
#define GL_DEBUG_LOGGED_MESSAGES 0x9145
#endif

#ifndef GL_DEBUG_SEVERITY_HIGH
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#endif

#ifndef GL_DEBUG_SEVERITY_MEDIUM
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#endif

#ifndef GL_DEBUG_SEVERITY_LOW
#define GL_DEBUG_SEVERITY_LOW 0x9148
#endif

#ifndef GL_DEBUG_TYPE_MARKER
#define GL_DEBUG_TYPE_MARKER 0x8268
#endif

#ifndef GL_DEBUG_TYPE_PUSH_GROUP
#define GL_DEBUG_TYPE_PUSH_GROUP 0x8269
#endif

#ifndef GL_DEBUG_TYPE_POP_GROUP
#define GL_DEBUG_TYPE_POP_GROUP 0x826A
#endif

#ifndef GL_DEBUG_SEVERITY_NOTIFICATION
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#endif

#ifndef GL_DEBUG_OUTPUT
#define GL_DEBUG_OUTPUT 0x92E0
#endif

#ifndef GL_DEBUG_OUTPUT_SYNCHRONOUS
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#endif


#ifndef GL_R32I
#define GL_R32I 0x8235
#endif
#ifndef GL_RED_INTEGER
#define GL_RED_INTEGER 0x8D94
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif
#ifndef GL_TEXTURE2
#define GL_TEXTURE2 0x84C2
#endif
#ifndef GL_TEXTURE3
#define GL_TEXTURE3 0x84C3
#endif
#ifndef GL_TEXTURE4
#define GL_TEXTURE4 0x84C4
#endif
#ifndef GL_TEXTURE5
#define GL_TEXTURE5 0x84C5
#endif
#ifndef GL_TEXTURE6
#define GL_TEXTURE6 0x84C6
#endif
#ifndef GL_TEXTURE7
#define GL_TEXTURE7 0x84C7
#endif
#ifndef GL_TEXTURE8
#define GL_TEXTURE8 0x84C8
#endif
#ifndef GL_TEXTURE9
#define GL_TEXTURE9 0x84C9
#endif
#ifndef GL_TEXTURE10
#define GL_TEXTURE10 0x84CA
#endif
#ifndef GL_TEXTURE11
#define GL_TEXTURE11 0x84CB
#endif
#ifndef GL_TEXTURE12
#define GL_TEXTURE12 0x84CC
#endif
#ifndef GL_TEXTURE13
#define GL_TEXTURE13 0x84CD
#endif
#ifndef GL_TEXTURE14
#define GL_TEXTURE14 0x84CE
#endif
#ifndef GL_TEXTURE15
#define GL_TEXTURE15 0x84CF
#endif
#ifndef GL_TEXTURE16
#define GL_TEXTURE16 0x84D0
#endif
#ifndef GL_TEXTURE17
#define GL_TEXTURE17 0x84D1
#endif
#ifndef GL_TEXTURE18
#define GL_TEXTURE18 0x84D2
#endif
#ifndef GL_TEXTURE19
#define GL_TEXTURE19 0x84D3
#endif
#ifndef GL_TEXTURE20
#define GL_TEXTURE20 0x84D4
#endif
#ifndef GL_TEXTURE21
#define GL_TEXTURE21 0x84D5
#endif
#ifndef GL_TEXTURE22
#define GL_TEXTURE22 0x84D6
#endif
#ifndef GL_TEXTURE23
#define GL_TEXTURE23 0x84D7
#endif
#ifndef GL_TEXTURE24
#define GL_TEXTURE24 0x84D8
#endif
#ifndef GL_TEXTURE25
#define GL_TEXTURE25 0x84D9
#endif
#ifndef GL_TEXTURE26
#define GL_TEXTURE26 0x84DA
#endif
#ifndef GL_TEXTURE27
#define GL_TEXTURE27 0x84DB
#endif
#ifndef GL_TEXTURE28
#define GL_TEXTURE28 0x84DC
#endif
#ifndef GL_TEXTURE29
#define GL_TEXTURE29 0x84DD
#endif
#ifndef GL_TEXTURE30
#define GL_TEXTURE30 0x84DE
#endif
#ifndef GL_TEXTURE31
#define GL_TEXTURE31 0x84DF
#endif
#ifndef GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#endif
#ifndef GL_TEXTURE_BUFFER
#define GL_TEXTURE_BUFFER 0x8C2A
#endif
#ifndef GL_RGBA32F
#define GL_RGBA32F 0x8814
#endif
#ifndef GL_RGB32F
#define GL_RGB32F 0x8815
#endif
#ifndef GL_RGBA16F
#define GL_RGBA16F 0x881A
#endif
#ifndef GL_RGB16F
#define GL_RGB16F 0x881B
#endif
#ifndef GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_MAX_IMAGE_UNITS
#define GL_MAX_IMAGE_UNITS 0x8F38
#endif
#ifndef GL_IMAGE_BINDING_NAME
#define GL_IMAGE_BINDING_NAME 0x8F3A
#endif

#ifndef GL_SHADER_STORAGE_BUFFER_BINDING
#define GL_SHADER_STORAGE_BUFFER_BINDING 0x90D3
#endif
#ifndef GL_BUFFER_SIZE
#define GL_BUFFER_SIZE 0x8764
#endif

#ifndef GL_MAX_COMPUTE_SHARED_MEMORY_SIZE
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#endif
#ifndef GL_MAX_COMPUTE_WORK_GROUP_SIZE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE 0x91BF
#endif
#ifndef GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
#endif


#ifndef GLchar
typedef char GLchar;
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

typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (APIENTRY* PFNGLVERTEXATTRIBDIVISORPROC)(GLuint index, GLuint divisor);
typedef void (APIENTRY* PFNGLDRAWARRAYSINSTANCEDPROC)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
typedef void (APIENTRY* PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint index);

typedef GLuint(APIENTRY* PFNGLCREATESHADERPROC)(GLenum type);
typedef void (APIENTRY* PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
typedef void (APIENTRY* PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef GLuint(APIENTRY* PFNGLCREATEPROGRAMPROC)();
typedef void (APIENTRY* PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (APIENTRY* PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (APIENTRY* PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void (APIENTRY* PFNGLDELETESHADERPROC)(GLuint shader);
typedef void (APIENTRY* PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void (APIENTRY* PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRY* PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRY* PFNGLUSEPROGRAMPROC)(GLuint program);
typedef GLint(APIENTRY* PFNGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
typedef void (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY* PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (APIENTRY* PFNUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void (APIENTRY* PFNUNIFORM4FVPROC)(GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRY* PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);

typedef void (APIENTRY* PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY* PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY* PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0, GLfloat v1);

typedef void* (APIENTRY* PFNGLMAPBUFFERPROC)(GLenum target, GLenum access);
typedef GLboolean(APIENTRY* PFNGLUNMAPBUFFERPROC)(GLenum target);

typedef void (APIENTRY* PFNGLUNIFORM2IPROC)(GLint location, GLint v0, GLint v1);
typedef void (APIENTRY* PFNGLBINDBUFFERBASEPROC)(GLenum target, GLuint index, GLuint buffer);
typedef void (APIENTRY* PFNGLDISPATCHCOMPUTEPROC)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void (APIENTRY* PFNGLMEMORYBARRIERPROC)(GLbitfield barriers);
typedef void (APIENTRY* PFNGLGETBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, void* data);

typedef void (APIENTRY* PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef const GLubyte* (APIENTRY* PFNGLGETSTRINGIPROC)(GLenum name, GLuint index);
typedef void (APIENTRY* PFNGLDRAWELEMENTSINSTANCEDPROC)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);

// Определение типов для указателей на функции отладки
typedef void (APIENTRY* GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
typedef void (APIENTRY* PFNGLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROC callback, const void* userParam);
typedef void (APIENTRY* PFNGLDEBUGMESSAGECONTROLPROC)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);

typedef void (APIENTRY* PFNGLBINDIMAGETEXTUREPROC)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (APIENTRY* PFNGLTEXBUFFERPROC)(GLenum target, GLenum internalformat, GLuint buffer);

typedef void (APIENTRY* PFNGLGETINTEGERI_VPROC)(GLenum target, GLuint index, GLint* data);
typedef void (APIENTRY* PFNGLGETBUFFERPARAMETERIVPROC)(GLenum target, GLenum pname, GLint* params);

typedef GLboolean(APIENTRY* PFNGLISBUFFERPROC)(GLuint buffer);

typedef void (APIENTRY* PFNGLUNIFORM1UIPROC)(GLint location, GLuint v0);

// Объявление указателей на функции
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
extern PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNUNIFORM1FPROC glUniform1f;
extern PFNUNIFORM4FVPROC glUniform4fv;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM4FPROC glUniform4f;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLMAPBUFFERPROC glMapBuffer;
extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;
extern PFNGLUNIFORM2IPROC glUniform2i;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;
extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
extern PFNGLMEMORYBARRIERPROC glMemoryBarrier;
extern PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLGETSTRINGIPROC glGetStringi;
extern PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;

extern PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
extern PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;

extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLTEXBUFFERPROC glTexBuffer;

extern PFNGLGETINTEGERI_VPROC glGetIntegeri_v;
extern PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv;

extern PFNGLISBUFFERPROC glIsBuffer;
extern PFNGLUNIFORM1UIPROC glUniform1ui;

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

#define CHECK_LOAD_FUNCTION(func) \
    std::cout << ((func) ? "            Успешно загружена функция " #func : "   Не удалось загрузить функцию " #func) << std::endl;
#endif //GLFUNC_H