#include "OpenGLInitializer.h"
#include "GLFunctions.h"

OpenGLInitializer::OpenGLInitializer(HWND hWnd) : hRC(NULL), hDC(NULL) {
    hDC = GetDC(hWnd);
}

OpenGLInitializer::~OpenGLInitializer() {
    if (hRC) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hRC);
    }
    if (hDC) {
        ReleaseDC(NULL, hDC);
    }
}

bool OpenGLInitializer::Initialize() {
    if (!SetupPixelFormat(hDC)) return false;

    hRC = wglCreateContext(hDC);
    if (!hRC) return false;

    if (!wglMakeCurrent(hDC, hRC)) return false;
    LoadOpenGLFunctions();
    return true;
}

bool OpenGLInitializer::SetupPixelFormat(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0) {
        DWORD error = GetLastError();
        wchar_t errorMessage[256];
        swprintf_s(errorMessage, sizeof(errorMessage) / sizeof(wchar_t), L"Не удалось выбрать pixel format. Код ошибки: %lu", error);
        MessageBox(NULL, errorMessage, L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        DWORD error = GetLastError();
        wchar_t errorMessage[256];
        swprintf_s(errorMessage, sizeof(errorMessage) / sizeof(wchar_t), L"Не удалось установить pixel format. Код ошибки: %lu", error);
        MessageBox(NULL, errorMessage, L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }


    return true;
}