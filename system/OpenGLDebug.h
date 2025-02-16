#pragma once
#ifndef OPENGL_DEBUG_H
#define OPENGL_DEBUG_H

#include "GLFunctions.h" // ���������� ������������ ��������� ��� OpenGL �������
#include <windows.h>
#include <DbgHelp.h>
#include <iostream>

#pragma comment(lib, "DbgHelp.lib") // ���������� ���������� DbgHelp

class OpenGLDebug {
public:
    OpenGLDebug();
    ~OpenGLDebug();

    // ������������� ����������� ������
    bool Initialize();

private:
    // �������-������� ��� ��������� ��������� OpenGL
    static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam);
};

#endif // OPENGL_DEBUG_H