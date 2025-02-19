#pragma once
#ifndef OPENGL_DEBUG_H
#define OPENGL_DEBUG_H

#include "GLFunctions.h" // Подключаем существующий заголовок для OpenGL функций
#include <windows.h>
#include <DbgHelp.h>
#include <iostream>

#pragma comment(lib, "DbgHelp.lib") // Подключаем библиотеку DbgHelp

class OpenGLDebug {
public:
    OpenGLDebug();
    ~OpenGLDebug();

    // Инициализация отладочного вывода
    bool Initialize();

private:
    // Коллбэк-функция для обработки сообщений OpenGL
    static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam);
};

#endif // OPENGL_DEBUG_H