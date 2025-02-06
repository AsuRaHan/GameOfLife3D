#include "OpenGLInitializer.h"
#include "GLFunctions.h"

OpenGLInitializer::OpenGLInitializer(HWND hWnd) : hRC(NULL), hDC(NULL) {
    m_hWnd = hWnd;
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

bool OpenGLInitializer::Initialize(bool fullScreen) {
    std::cout << "������� ������������� OpenGL" << std::endl;

    // ��������� �������������� ������, ���� ���������
    if (fullScreen) {
        // ������� ����� ���� � ���������� � ������
        LONG_PTR style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
        style &= ~WS_CAPTION; // ������� ���������
        style &= ~WS_THICKFRAME; // ������� ����� (��� ���������� ����)
        style &= ~WS_BORDER; // ������� ������� ������� ��� ������������ ����
        style &= ~WS_SIZEBOX; // ������� ��������� �����

        SetWindowLongPtr(m_hWnd, GWL_STYLE, style);

        // ����� ����� �������� ����������� ����� ��� ������� �������� ���� ���������
        LONG_PTR exStyle = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
        exStyle &= ~WS_EX_WINDOWEDGE;
        exStyle &= ~WS_EX_CLIENTEDGE;
        exStyle &= ~WS_EX_STATICEDGE;

        SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, exStyle);

        // ������������ ����, ����� ��������� �������� � ����
        //SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
        dmScreenSettings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        SetWindowPos(m_hWnd, HWND_TOP, 0, 0, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight, SWP_SHOWWINDOW);
        ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);

        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            std::cout << "�� ������� ������������� � ������������� �����" << std::endl;
            return false;
        }
    }
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"OpenGLTempClass";
    if (!RegisterClassEx(&wc)) {
        std::cout << "�� ������� ���������������� ����� ����. ��� ������: " << GetLastError() << std::endl;
        return false;
    }

    // ��������� ���� ��� ������������� OpenGL
    HWND temporaryWindow = CreateWindowEx(WS_EX_APPWINDOW, L"OpenGLTempClass", L"Temp", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (temporaryWindow == NULL) {
        std::cout << "�� ������� ������� ��������� ����. ��� ������: " << GetLastError() << std::endl;
        return false;
    }
    HDC temporaryDC = GetDC(temporaryWindow);
    if (temporaryDC == NULL) {
        std::cout << "�� ������� �������� HDC ��� ���������� ����. ��� ������: " << GetLastError() << std::endl;
        DestroyWindow(temporaryWindow);
        return false;
    }
    if (!SetupPixelFormat(temporaryDC)) {
        DestroyWindow(temporaryWindow);
        return false;
    }
    std::cout << "          SetupPixelFormat �������" << std::endl;

    HGLRC temporaryRC = wglCreateContext(temporaryDC);
    wglMakeCurrent(temporaryDC, temporaryRC);

    // �������� ���������� OpenGL
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
        std::cout << "�� ������� ��������� OpenGL ����������" << std::endl;
        wglDeleteContext(temporaryRC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    // ����� ������� ��� ��������� ����
    const int formatAttributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        0
    };

    const int contextAttributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    int format, formatCount;
    wglChoosePixelFormatARB(hDC, formatAttributes, NULL, 1, &format, (UINT*)&formatCount);
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(hDC, format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    SetPixelFormat(hDC, format, &pfd);

    // �������� ������������ ��������� OpenGL
    hRC = wglCreateContextAttribsARB(hDC, NULL, contextAttributes);
    if (!hRC) {
        std::cout << "�� ������� ������� �������� OpenGL" << std::endl;
        wglDeleteContext(temporaryRC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    wglMakeCurrent(hDC, hRC);
    wglDeleteContext(temporaryRC);
    ReleaseDC(temporaryWindow, temporaryDC);
    DestroyWindow(temporaryWindow);

    std::cout << "          wglCreateContext �������" << std::endl;
    std::cout << "          wglMakeCurrent �������" << std::endl;

    std::cout << "������� ������ �������� OpenGL �������" << std::endl;
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
        std::cout << "�� ������� ������� pixel format. ��� ������: " << error << std::endl;
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        DWORD error = GetLastError();
        std::cout << "�� ������� ���������� pixel format. ��� ������: " << error << std::endl;
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