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
    std::cout << "Начинаю инициализацию OpenGL" << std::endl;
    // Настройка полноэкранного режима, если требуется
    if (fullScreen) {
        // Удаляем стиль окна с заголовком и рамкой
        LONG_PTR style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
        style &= ~WS_CAPTION; // Убирает заголовок
        style &= ~WS_THICKFRAME; // Убирает рамку (для изменяемых окон)
        style &= ~WS_BORDER; // Убирает простую границу для неизменяемых окон
        style &= ~WS_SIZEBOX; // Убирает размерную рамку

        //if (
        SetWindowLongPtr(m_hWnd, GWL_STYLE, style);
        //    ) {
        //    std::cout << "Не удалось изменить стиль окна: " << GetErrorMessage(GetLastError()) << std::endl;
        //    return false;
        //}

        // Изменяем расширенный стиль
        LONG_PTR exStyle = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
        exStyle &= ~WS_EX_WINDOWEDGE;
        exStyle &= ~WS_EX_CLIENTEDGE;
        exStyle &= ~WS_EX_STATICEDGE;

        //if (
        SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, exStyle);
        //    ) {
        //    std::cout << "Не удалось изменить расширенный стиль окна: " << GetErrorMessage(GetLastError()) << std::endl;
        //    return false;
        //}

        // Перерисовать окно
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

    // Регистрация класса окна
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, DefWindowProc, 0, 0, GetModuleHandle(nullptr), nullptr, nullptr, (HBRUSH)(COLOR_WINDOW + 1), nullptr, L"OpenGLTempClass", nullptr };
    if (!RegisterClassEx(&wc)) {
        std::cout << "Не удалось зарегистрировать класс временного окна: " << GetErrorMessage(GetLastError()) << std::endl;
        return false;
    }

    // Создание временного окна
    HWND temporaryWindow = CreateWindowEx(WS_EX_APPWINDOW, L"OpenGLTempClass", L"Temp", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(nullptr), NULL);
    if (temporaryWindow == nullptr) {
        std::cout << "Не удалось создать временное окно: " << GetErrorMessage(GetLastError()) << std::endl;
        return false;
    }

    HDC temporaryDC = GetDC(temporaryWindow);
    if (temporaryDC == nullptr) {
        std::cout << "Не удалось получить HDC для временного окна: " << GetErrorMessage(GetLastError()) << std::endl;
        DestroyWindow(temporaryWindow);
        return false;
    }

    if (!SetupPixelFormat(temporaryDC)) {
        std::cout << "Не удалось установить формат пикселей для временного окна" << std::endl;
        ReleaseDC(temporaryWindow, temporaryDC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    HGLRC temporaryRC = wglCreateContext(temporaryDC);
    if (temporaryRC == nullptr) {
        std::cout << "Не удалось создать временный контекст OpenGL: " << GetErrorMessage(GetLastError()) << std::endl;
        ReleaseDC(temporaryWindow, temporaryDC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    wglMakeCurrent(temporaryDC, temporaryRC);

    // Загрузка расширений OpenGL
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
        std::cout << "Не удалось загрузить OpenGL расширения" << std::endl;
        wglDeleteContext(temporaryRC);
        ReleaseDC(temporaryWindow, temporaryDC);
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
    int major, minor;
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "  OpenGL Version: " << version << std::endl;
    // Извлекаем основные и минорные версии OpenGL
    std::istringstream versionStream(reinterpret_cast<const char*>(version));
    versionStream >> major; // Основная версия
    versionStream.ignore(1); // Игнорируем точку
    versionStream >> minor; // Минорная версия

    const int contextAttributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    //====================================
    if (glGetString(GL_SHADING_LANGUAGE_VERSION) == nullptr) {
        std::cout << "Не удалось получить версию GLSL" << std::endl;
    }
    else {
        std::cout << "  GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    }
    // Проверка поддержки теселяции
    if (glGetString(GL_EXTENSIONS) != nullptr) {
        std::string extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        if (extensions.find("GL_ARB_tessellation_shader") != std::string::npos) {
            std::cout << "  Теселяция поддерживается" << std::endl;
        }
        else {
            std::cout << "  Теселяция не поддерживается" << std::endl;
        }
    }
    //====================================
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

    std::cout << "  wglCreateContext успешно" << std::endl;
    std::cout << "  wglMakeCurrent успешно" << std::endl;

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
        std::cout << "Не удалось выбрать pixel format: " << GetErrorMessage(GetLastError()) << std::endl;
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        std::cout << "Не удалось установить pixel format: " << GetErrorMessage(GetLastError()) << std::endl;
        return false;
    }

    return true;
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

std::string OpenGLInitializer::GetErrorMessage(DWORD errorCode) {
    LPVOID lpMsgBuf;
    DWORD bufLen;

    // Получаем сообщение об ошибке
    bufLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Используем язык по умолчанию
        (LPWSTR)&lpMsgBuf,
        0,
        NULL);

    std::string message;
    if (bufLen > 0) {
        // Преобразуем wide string в narrow string
        wchar_t* wideMessage = (wchar_t*)lpMsgBuf;
        int size_needed = WideCharToMultiByte(CP_ACP, 0, wideMessage, -1, NULL, 0, NULL, NULL);
        message.resize(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, wideMessage, -1, &message[0], size_needed, NULL, NULL);
        LocalFree(lpMsgBuf); // Освобождаем память
    }
    else {
        // Если bufLen равно 0, значит произошла ошибка
        DWORD error = GetLastError();
        message = "Не удалось получить сообщение об ошибке. Код ошибки: " + std::to_string(error);
    }

    return message;
}