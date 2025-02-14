#pragma once
#ifndef OPENGLINITIALIZER_H_
#define OPENGLINITIALIZER_H_

#pragma once
#include <Windows.h>
#include <gl\GL.h>
#include <gl\glu.h>
#include <cstdio> // For swprintf and swprintf_s
#include <sstream>

//#include "GLFunctions.h"

class OpenGLInitializer {
private:
    HGLRC hRC;
    HDC hDC;
    HWND m_hWnd;
    // Указатели на функции расширений
    typedef BOOL(APIENTRY* PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int*, const FLOAT*, UINT, int*, UINT*);
    typedef HGLRC(APIENTRY* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
public:
    OpenGLInitializer(HWND hWnd);
    ~OpenGLInitializer();
    bool Initialize(bool fullScreen=false);

private:
    bool SetupPixelFormat(HDC hdc);
    bool checkForDepthIssues();
    float testFarPlane(float start, float end, float step);
    void printSystemInfo();
    std::string GetErrorMessage(DWORD errorCode);
};
#endif // OPENGLINITIALIZER_H_