#include "OpenGLInitializer.h"
#include "GLFunctions.h"

OpenGLInitializer::OpenGLInitializer(HWND hWnd) : hRC(NULL), hDC(NULL),
wglChoosePixelFormatARB(nullptr), wglCreateContextAttribsARB(nullptr)
{
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

        //if (
        SetWindowLongPtr(m_hWnd, GWL_STYLE, style);
        //    ) {
        //    std::cout << "�� ������� �������� ����� ����: " << GetErrorMessage(GetLastError()) << std::endl;
        //    return false;
        //}

        // �������� ����������� �����
        LONG_PTR exStyle = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
        exStyle &= ~WS_EX_WINDOWEDGE;
        exStyle &= ~WS_EX_CLIENTEDGE;
        exStyle &= ~WS_EX_STATICEDGE;

        //if (
        SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, exStyle);
        //    ) {
        //    std::cout << "�� ������� �������� ����������� ����� ����: " << GetErrorMessage(GetLastError()) << std::endl;
        //    return false;
        //}

        // ������������ ����
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

    // ����������� ������ ����
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, DefWindowProc, 0, 0, GetModuleHandle(nullptr), nullptr, nullptr, (HBRUSH)(COLOR_WINDOW + 1), nullptr, L"OpenGLTempClass", nullptr };
    if (!RegisterClassEx(&wc)) {
        std::cout << "�� ������� ���������������� ����� ���������� ����: " << GetErrorMessage(GetLastError()) << std::endl;
        return false;
    }

    // �������� ���������� ����
    HWND temporaryWindow = CreateWindowEx(WS_EX_APPWINDOW, L"OpenGLTempClass", L"Temp", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(nullptr), NULL);
    if (temporaryWindow == nullptr) {
        std::cout << "�� ������� ������� ��������� ����: " << GetErrorMessage(GetLastError()) << std::endl;
        return false;
    }

    HDC temporaryDC = GetDC(temporaryWindow);
    if (temporaryDC == nullptr) {
        std::cout << "�� ������� �������� HDC ��� ���������� ����: " << GetErrorMessage(GetLastError()) << std::endl;
        DestroyWindow(temporaryWindow);
        return false;
    }

    if (!SetupPixelFormat(temporaryDC)) {
        std::cout << "�� ������� ���������� ������ �������� ��� ���������� ����" << std::endl;
        ReleaseDC(temporaryWindow, temporaryDC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    HGLRC temporaryRC = wglCreateContext(temporaryDC);
    if (temporaryRC == nullptr) {
        std::cout << "�� ������� ������� ��������� �������� OpenGL: " << GetErrorMessage(GetLastError()) << std::endl;
        ReleaseDC(temporaryWindow, temporaryDC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    wglMakeCurrent(temporaryDC, temporaryRC);

    // �������� ���������� OpenGL
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
        std::cout << "�� ������� ��������� OpenGL ����������" << std::endl;
        wglDeleteContext(temporaryRC);
        ReleaseDC(temporaryWindow, temporaryDC);
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
    int major, minor;
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "  OpenGL Version: " << version << std::endl;
    // ��������� �������� � �������� ������ OpenGL
    std::istringstream versionStream(reinterpret_cast<const char*>(version));
    versionStream >> major; // �������� ������
    versionStream.ignore(1); // ���������� �����
    versionStream >> minor; // �������� ������

    const int contextAttributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    //====================================
    if (glGetString(GL_SHADING_LANGUAGE_VERSION) == nullptr) {
        std::cout << "�� ������� �������� ������ GLSL" << std::endl;
    }
    else {
        std::cout << "  GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    }
    // �������� ��������� ���������
    if (glGetString(GL_EXTENSIONS) != nullptr) {
        std::string extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        if (extensions.find("GL_ARB_tessellation_shader") != std::string::npos) {
            std::cout << "  ��������� ��������������" << std::endl;
        }
        else {
            std::cout << "  ��������� �� ��������������" << std::endl;
        }
    }
    //====================================
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

    std::cout << "  wglCreateContext �������" << std::endl;
    std::cout << "  wglMakeCurrent �������" << std::endl;

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
        std::cout << "�� ������� ������� pixel format: " << GetErrorMessage(GetLastError()) << std::endl;
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        std::cout << "�� ������� ���������� pixel format: " << GetErrorMessage(GetLastError()) << std::endl;
        return false;
    }

    return true;
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

std::string OpenGLInitializer::GetErrorMessage(DWORD errorCode) {
    LPVOID lpMsgBuf;
    DWORD bufLen;

    // �������� ��������� �� ������
    bufLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // ���������� ���� �� ���������
        (LPWSTR)&lpMsgBuf,
        0,
        NULL);

    std::string message;
    if (bufLen > 0) {
        // ����������� wide string � narrow string
        wchar_t* wideMessage = (wchar_t*)lpMsgBuf;
        int size_needed = WideCharToMultiByte(CP_ACP, 0, wideMessage, -1, NULL, 0, NULL, NULL);
        message.resize(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, wideMessage, -1, &message[0], size_needed, NULL, NULL);
        LocalFree(lpMsgBuf); // ����������� ������
    }
    else {
        // ���� bufLen ����� 0, ������ ��������� ������
        DWORD error = GetLastError();
        message = "�� ������� �������� ��������� �� ������. ��� ������: " + std::to_string(error);
    }

    return message;
}