#include "GLFunctions.h"

// Определение указателей на функции
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;

void LoadOpenGLFunctions() {
    // Загружаем функции
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");

    // Проверяем, что все функции успешно загружены
    if (!glGenBuffers || !glBindBuffer || !glBufferData || !glDeleteBuffers || !glBufferSubData) {
        MessageBox(NULL, L"Неполучилось загрузить указатели на OpenGL функции.", L"Ошибка", MB_OK | MB_ICONERROR);
        exit(1); // или другой способ обработки ошибки
    }
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