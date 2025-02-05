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
    std::cout << "Начинаю инициализацию OpenGL" << std::endl;
    if (!SetupPixelFormat(hDC)) return false;
    std::cout << "          SetupPixelFormat успешно" << std::endl;
    hRC = wglCreateContext(hDC);
    if (!hRC) return false;
    std::cout << "          wglCreateContext успешно" << std::endl;
    if (!wglMakeCurrent(hDC, hRC)) return false;
    std::cout << "          wglMakeCurrent успешно" << std::endl;
    std::cout << "Начинаю ручную загрузку OpenGL функций" << std::endl;
    LoadOpenGLFunctions();
    //printSystemInfo();
    //float maxFarPlane = testFarPlane(10.0f, 1000000000000.0f, 1000000000.0f);
    //std::cout << "Максимальное значение farPlane без проблем: " << maxFarPlane << std::endl;
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
        swprintf_s(errorMessage, sizeof(errorMessage) / sizeof(wchar_t), L"Не удалось выбрать pixel format. Код ошибки: %lu", error);
        MessageBox(NULL, errorMessage, L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        DWORD error = GetLastError();
        wchar_t errorMessage[256];
        swprintf_s(errorMessage, sizeof(errorMessage) / sizeof(wchar_t), L"Не удалось установить pixel format. Код ошибки: %lu", error);
        MessageBox(NULL, errorMessage, L"Ошибка", MB_OK | MB_ICONERROR);
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