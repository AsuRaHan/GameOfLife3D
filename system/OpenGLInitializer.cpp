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
    std::cout << "Начинаю инициализацию OpenGL" << std::endl;

    // Настройка полноэкранного режима, если требуется
    if (fullScreen) {
        // Удаляем стиль окна с заголовком и рамкой
        LONG_PTR style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
        style &= ~WS_CAPTION; // Убирает заголовок
        style &= ~WS_THICKFRAME; // Убирает рамку (для изменяемых окон)
        style &= ~WS_BORDER; // Убирает простую границу для неизменяемых окон
        style &= ~WS_SIZEBOX; // Убирает размерную рамку

        SetWindowLongPtr(m_hWnd, GWL_STYLE, style);

        // Также можно изменить расширенный стиль для полного удаления всех декораций
        LONG_PTR exStyle = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
        exStyle &= ~WS_EX_WINDOWEDGE;
        exStyle &= ~WS_EX_CLIENTEDGE;
        exStyle &= ~WS_EX_STATICEDGE;

        SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, exStyle);

        // Перерисовать окно, чтобы изменения вступили в силу
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
            std::cout << "Не удалось переключиться в полноэкранный режим" << std::endl;
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
        std::cout << "Не удалось зарегистрировать класс окна. Код ошибки: " << GetLastError() << std::endl;
        return false;
    }

    // Временное окно для инициализации OpenGL
    HWND temporaryWindow = CreateWindowEx(WS_EX_APPWINDOW, L"OpenGLTempClass", L"Temp", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (temporaryWindow == NULL) {
        std::cout << "Не удалось создать временное окно. Код ошибки: " << GetLastError() << std::endl;
        return false;
    }
    HDC temporaryDC = GetDC(temporaryWindow);
    if (temporaryDC == NULL) {
        std::cout << "Не удалось получить HDC для временного окна. Код ошибки: " << GetLastError() << std::endl;
        DestroyWindow(temporaryWindow);
        return false;
    }
    if (!SetupPixelFormat(temporaryDC)) {
        DestroyWindow(temporaryWindow);
        return false;
    }
    std::cout << "          SetupPixelFormat успешно" << std::endl;

    HGLRC temporaryRC = wglCreateContext(temporaryDC);
    wglMakeCurrent(temporaryDC, temporaryRC);

    // Загрузка расширений OpenGL
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
        std::cout << "Не удалось загрузить OpenGL расширения" << std::endl;
        wglDeleteContext(temporaryRC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    // Выбор формата для основного окна
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

    // Создание современного контекста OpenGL
    hRC = wglCreateContextAttribsARB(hDC, NULL, contextAttributes);
    if (!hRC) {
        std::cout << "Не удалось создать контекст OpenGL" << std::endl;
        wglDeleteContext(temporaryRC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    wglMakeCurrent(hDC, hRC);
    wglDeleteContext(temporaryRC);
    ReleaseDC(temporaryWindow, temporaryDC);
    DestroyWindow(temporaryWindow);

    std::cout << "          wglCreateContext успешно" << std::endl;
    std::cout << "          wglMakeCurrent успешно" << std::endl;

    std::cout << "Начинаю ручную загрузку OpenGL функций" << std::endl;
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
        std::cout << "Не удалось выбрать pixel format. Код ошибки: " << error << std::endl;
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        DWORD error = GetLastError();
        std::cout << "Не удалось установить pixel format. Код ошибки: " << error << std::endl;
        return false;
    }

    return true;
}

float OpenGLInitializer::testFarPlane(float start, float end, float step) {
    float farPlane = start;
    bool hasIssues = false;

    while (farPlane <= end) {
        // Установите farPlane
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0f, 1.0f, 0.1f, farPlane); // fov, aspect, near, far

        // Здесь вы должны выполнить рендеринг сцены и проверить на наличие артефактов
        // Например, вы можете отрисовать объекты и проверить их видимость

        // Для упрощения, предположим, что у вас есть функция, которая проверяет наличие проблем
        hasIssues = checkForDepthIssues(); // Эта функция должна быть реализована вами

        if (hasIssues) {
            break; // Если есть проблемы, выходим из цикла
        }

        farPlane += step; // Увеличиваем farPlane
    }

    return farPlane - step; // Возвращаем последнее значение без проблем
}

bool OpenGLInitializer::checkForDepthIssues() {
    // Получаем размер окна
    int width = 800; // Замените на фактическую ширину вашего окна
    int height = 600; // Замените на фактическую высоту вашего окна

    // Создаем буфер для хранения значений глубины
    GLfloat* depthBuffer = new GLfloat[width * height];

    // Читаем значения глубины из буфера
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer);

    // Пример простой проверки: если значение глубины в центре экрана слишком близко к 1.0 (что соответствует farPlane)
    // Это может указывать на проблемы с глубиной
    GLfloat centerDepth = depthBuffer[(height / 2) * width + (width / 2)];

    delete[] depthBuffer;

    // Если значение глубины слишком близко к farPlane, это может быть проблемой
    if (centerDepth > 0.9f) {
        return true; // Проблема с глубиной
    }

    return false; // Проблем нет
}

void OpenGLInitializer::printSystemInfo() {
    // Получаем информацию о видеокарте
    const GLubyte* renderer = glGetString(GL_RENDERER); // Название видеокарты
    const GLubyte* vendor = glGetString(GL_VENDOR);     // Производитель видеокарты
    const GLubyte* version = glGetString(GL_VERSION);   // Версия OpenGL
    const GLubyte* shadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION); // Версия шейдерного языка

    // Выводим информацию
    std::cout << "Видеокарта:               " << renderer << std::endl;
    std::cout << "Производитель:            " << vendor << std::endl;
    std::cout << "Версия OpenGL:            " << version << std::endl;
    std::cout << "Версия шейдерного языка:  " << shadingLanguageVersion << std::endl;

    // Получаем информацию о поддерживаемых расширениях
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    std::cout << "Поддерживаемые расширения: " << std::endl;
    for (GLint i = 0; i < numExtensions; ++i) {
        std::cout << "      " << glGetStringi(GL_EXTENSIONS, i) << std::endl;
    }
}