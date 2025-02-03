#include "GLFunctions.h"

// ���������� ���������� �� �������
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
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;

PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLUNIFORM4FPROC glUniform4f = nullptr;
PFNGLUNIFORM2FPROC glUniform2f = nullptr;

PFNGLMAPBUFFERPROC glMapBuffer = nullptr;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = nullptr;

void LoadOpenGLFunctions() {
    // �������� ������� ��� ������ � ��������
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
    // �������� ������� ��� �����������
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
    // �������� ������� ��� ������ � ���������
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
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
    CHECK_LOAD_FUNCTION(glDeleteVertexArrays);

    glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
    CHECK_LOAD_FUNCTION(glUniform3f);
    glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
    CHECK_LOAD_FUNCTION(glUniform4f);
    glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
    CHECK_LOAD_FUNCTION(glUniform2f);

    glMapBuffer = (PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
    CHECK_LOAD_FUNCTION(glMapBuffer);
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
    CHECK_LOAD_FUNCTION(glUnmapBuffer);
}

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    static int errorCount = 0; // ����������� ���������� ��� ������������ ���������� ������
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        errorCount++; // ����������� ������� ������
        PlaySound(TEXT("SystemExclamation"), NULL, SND_ALIAS | SND_ASYNC);
        Sleep(1000); // �������� 1 �������
        std::cout << "OpenGL error " << std::hex << err
            << ", at " << fname << ":" << line
            << " - for " << stmt << std::endl;
        // ���������, �������� �� �� 10 ������
        if (errorCount >= 10)
        {
            std::cout << "���������� ������������ ���������� ������. ���������� ���������." << std::endl;
            WCHAR errorMsg[256];
            swprintf_s(errorMsg, L"OpenGL error %08X, at %S:%d - for %S", err, fname, line, stmt);
            MessageBoxW(NULL, errorMsg, L"OpenGL Error", MB_OK | MB_ICONERROR);

            exit(EXIT_FAILURE); // ��������� ���������
        }
    }
}