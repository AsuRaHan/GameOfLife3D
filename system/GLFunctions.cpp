#include "GLFunctions.h"

// Объявление указателей на функции
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;

PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor = nullptr;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;

PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;

PFNGLUSEPROGRAMPROC glUseProgram = nullptr;

PFNGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;

PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;

PFNUNIFORM1FPROC glUniform1f = nullptr;
PFNUNIFORM4FVPROC glUniform4fv = nullptr;


void LoadOpenGLFunctions() {
    // Загрузка функций для работы с буферами
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    CHECK_LOAD_FUNCTION(glGenBuffers);
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    CHECK_LOAD_FUNCTION(glBindBuffer);
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    CHECK_LOAD_FUNCTION(glBufferData);
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    CHECK_LOAD_FUNCTION(glDeleteBuffers);
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
    CHECK_LOAD_FUNCTION(glBufferSubData);
    // Загрузка функций для инстансинга
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    CHECK_LOAD_FUNCTION(glEnableVertexAttribArray);
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    CHECK_LOAD_FUNCTION(glVertexAttribPointer);
    glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)wglGetProcAddress("glVertexAttribDivisor");
    CHECK_LOAD_FUNCTION(glVertexAttribDivisor);
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)wglGetProcAddress("glDrawArraysInstanced");
    CHECK_LOAD_FUNCTION(glDrawArraysInstanced);
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
    CHECK_LOAD_FUNCTION(glDisableVertexAttribArray);
    // Загрузка функций для работы с шейдерами
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    CHECK_LOAD_FUNCTION(glCreateShader);
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    CHECK_LOAD_FUNCTION(glShaderSource);
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    CHECK_LOAD_FUNCTION(glCompileShader);
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    CHECK_LOAD_FUNCTION(glCreateProgram);
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    CHECK_LOAD_FUNCTION(glAttachShader);
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    CHECK_LOAD_FUNCTION(glLinkProgram);
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    CHECK_LOAD_FUNCTION(glDeleteProgram);
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    CHECK_LOAD_FUNCTION(glDeleteShader);
    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    CHECK_LOAD_FUNCTION(glGetShaderiv);
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    CHECK_LOAD_FUNCTION(glGetShaderInfoLog);
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    CHECK_LOAD_FUNCTION(glGetProgramiv);
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    CHECK_LOAD_FUNCTION(glGetProgramInfoLog);
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    CHECK_LOAD_FUNCTION(glUseProgram);

    glGetUniformLocation = (PFNGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    CHECK_LOAD_FUNCTION(glGetUniformLocation);
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    CHECK_LOAD_FUNCTION(glUniformMatrix4fv);

    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
    CHECK_LOAD_FUNCTION(glBindVertexArray);
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
    CHECK_LOAD_FUNCTION(glGenVertexArrays);

    glUniform1f = (PFNUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
    CHECK_LOAD_FUNCTION(glUniform1f);

    glUniform4fv = (PFNUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
    CHECK_LOAD_FUNCTION(glUniform4fv);

    // Проверка на успешную загрузку всех функций
    //bool allLoaded = glGenBuffers && glBindBuffer && glBufferData && glDeleteBuffers && glBufferSubData &&
    //    glEnableVertexAttribArray && glVertexAttribPointer && glVertexAttribDivisor &&
    //    glDrawArraysInstanced && glDisableVertexAttribArray &&
    //    glCreateShader && glShaderSource && glCompileShader && glCreateProgram &&
    //    glAttachShader && glLinkProgram && glDeleteProgram && glDeleteShader &&
    //    glGetShaderiv && glGetShaderInfoLog && glGetProgramiv && glGetProgramInfoLog &&
    //    glUseProgram && glGetUniformLocation && glUniformMatrix4fv;

    //if (!allLoaded) {
    //        MessageBox(NULL, L"Не удалось загрузить указатели на некоторые функции OpenGL.", L"Ошибка", MB_OK | MB_ICONERROR);
    //    exit(1);
    //}
}

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        WCHAR errorMsg[256];
        swprintf_s(errorMsg, L"OpenGL error %08X, at %S:%d - for %S", err, fname, line, stmt);

        MessageBoxW(NULL, errorMsg, L"OpenGL Error", MB_OK | MB_ICONERROR);
        // Можно добавить дополнительную обработку ошибок или выход из программы
    }
}