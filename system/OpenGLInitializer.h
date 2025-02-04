#ifndef OPENGLINITIALIZER_H_
#define OPENGLINITIALIZER_H_

#pragma once
#include <Windows.h>
#include <gl\GL.h>
#include <cstdio> // For swprintf and swprintf_s

//#include "GLFunctions.h"

class OpenGLInitializer {
private:
    HGLRC hRC;
    HDC hDC;

public:
    OpenGLInitializer(HWND hWnd);
    ~OpenGLInitializer();
    bool Initialize();

private:
    bool SetupPixelFormat(HDC hdc);
};
#endif // OPENGLINITIALIZER_H_