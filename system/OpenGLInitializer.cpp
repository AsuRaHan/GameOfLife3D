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
    std::cout << "������� ������������� OpenGL" << std::endl;
    if (!SetupPixelFormat(hDC)) return false;
    std::cout << "          SetupPixelFormat �������" << std::endl;
    hRC = wglCreateContext(hDC);
    if (!hRC) return false;
    std::cout << "          wglCreateContext �������" << std::endl;
    if (!wglMakeCurrent(hDC, hRC)) return false;
    std::cout << "          wglMakeCurrent �������" << std::endl;
    std::cout << "������� ������ �������� OpenGL �������" << std::endl;
    LoadOpenGLFunctions();
    //printSystemInfo();
    //float maxFarPlane = testFarPlane(10.0f, 1000000000000.0f, 1000000000.0f);
    //std::cout << "������������ �������� farPlane ��� �������: " << maxFarPlane << std::endl;
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
        swprintf_s(errorMessage, sizeof(errorMessage) / sizeof(wchar_t), L"�� ������� ������� pixel format. ��� ������: %lu", error);
        MessageBox(NULL, errorMessage, L"������", MB_OK | MB_ICONERROR);
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        DWORD error = GetLastError();
        wchar_t errorMessage[256];
        swprintf_s(errorMessage, sizeof(errorMessage) / sizeof(wchar_t), L"�� ������� ���������� pixel format. ��� ������: %lu", error);
        MessageBox(NULL, errorMessage, L"������", MB_OK | MB_ICONERROR);
        return false;
    }


    return true;
}

float OpenGLInitializer::testFarPlane(float start, float end, float step) {
    float farPlane = start;
    bool hasIssues = false;

    while (farPlane <= end) {
        // ���������� farPlane
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0f, 1.0f, 0.1f, farPlane); // fov, aspect, near, far

        // ����� �� ������ ��������� ��������� ����� � ��������� �� ������� ����������
        // ��������, �� ������ ���������� ������� � ��������� �� ���������

        // ��� ���������, �����������, ��� � ��� ���� �������, ������� ��������� ������� �������
        hasIssues = checkForDepthIssues(); // ��� ������� ������ ���� ����������� ����

        if (hasIssues) {
            break; // ���� ���� ��������, ������� �� �����
        }

        farPlane += step; // ����������� farPlane
    }

    return farPlane - step; // ���������� ��������� �������� ��� �������
}

bool OpenGLInitializer::checkForDepthIssues() {
    // �������� ������ ����
    int width = 800; // �������� �� ����������� ������ ������ ����
    int height = 600; // �������� �� ����������� ������ ������ ����

    // ������� ����� ��� �������� �������� �������
    GLfloat* depthBuffer = new GLfloat[width * height];

    // ������ �������� ������� �� ������
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer);

    // ������ ������� ��������: ���� �������� ������� � ������ ������ ������� ������ � 1.0 (��� ������������� farPlane)
    // ��� ����� ��������� �� �������� � ��������
    GLfloat centerDepth = depthBuffer[(height / 2) * width + (width / 2)];

    delete[] depthBuffer;

    // ���� �������� ������� ������� ������ � farPlane, ��� ����� ���� ���������
    if (centerDepth > 0.9f) {
        return true; // �������� � ��������
    }

    return false; // ������� ���
}

void OpenGLInitializer::printSystemInfo() {
    // �������� ���������� � ����������
    const GLubyte* renderer = glGetString(GL_RENDERER); // �������� ����������
    const GLubyte* vendor = glGetString(GL_VENDOR);     // ������������� ����������
    const GLubyte* version = glGetString(GL_VERSION);   // ������ OpenGL
    const GLubyte* shadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION); // ������ ���������� �����

    // ������� ����������
    std::cout << "����������:               " << renderer << std::endl;
    std::cout << "�������������:            " << vendor << std::endl;
    std::cout << "������ OpenGL:            " << version << std::endl;
    std::cout << "������ ���������� �����:  " << shadingLanguageVersion << std::endl;

    // �������� ���������� � �������������� �����������
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    std::cout << "�������������� ����������: " << std::endl;
    for (GLint i = 0; i < numExtensions; ++i) {
        std::cout << "      " << glGetStringi(GL_EXTENSIONS, i) << std::endl;
    }
}