#include "OpenGLDebug.h"

OpenGLDebug::OpenGLDebug() {
}

OpenGLDebug::~OpenGLDebug() {
}

bool OpenGLDebug::Initialize() {
    std::cout << "Инициализация отладки OpenGL" << std::endl;

    // Проверяем, что функции загружены
    if (!glDebugMessageCallback || !glDebugMessageControl) {
        std::cout << "Не удалось загрузить функции отладки OpenGL" << std::endl;
        return false;
    }

    // Устанавливаем коллбэк
    GL_CHECK(glDebugMessageCallback(DebugCallback, nullptr));

    // Включаем отладочный вывод
    GL_CHECK(glEnable(GL_DEBUG_OUTPUT));
    GL_CHECK(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS)); // Синхронный вывод для точного стека вызовов

    // Фильтрация сообщений (опционально)
    // Отключаем сообщения низкой серьезности
    GL_CHECK(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE));


    std::cout << "Отладка OpenGL успешно инициализирована" << std::endl;
    return true;
}

void APIENTRY OpenGLDebug::DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam) {
    // Преобразуем перечисления в читаемые строки
    const char* sourceStr;
    switch (source) {
    case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
    default:                              sourceStr = "Unknown"; break;
    }

    const char* typeStr;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    default:                                typeStr = "Unknown"; break;
    }

    const char* severityStr;
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    default:                             severityStr = "Unknown"; break;
    }

    // Захватываем стек вызовов
    const int maxFrames = 16;
    void* stack[maxFrames];
    USHORT frames = CaptureStackBackTrace(0, maxFrames, stack, NULL);

    // Инициализируем символы для трассировки
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    std::cout << "OpenGL Debug Message:" << std::endl
        << "  Source: " << sourceStr << std::endl
        << "  Type: " << typeStr << std::endl
        << "  ID: " << id << std::endl
        << "  Severity: " << severityStr << std::endl
        << "  Message: " << message << std::endl
        << "  Stack Trace:" << std::endl;

    // Выводим трассировку стека
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (USHORT i = 0; i < frames; i++) {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
        std::cout << "    " << i << ": " << symbol->Name << " - 0x" << std::hex << symbol->Address << std::dec << std::endl;
    }

    free(symbol);
    SymCleanup(process);

    // Если ошибка критическая, вызываем MessageBox и завершаем программу
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        WCHAR wMessage[1024];
        swprintf_s(wMessage, L"[OpenGLDebug] OpenGL Error:\nSource: %S\nType: %S\nID: %u\nSeverity: %S\nMessage: %S",
            sourceStr, typeStr, id, severityStr, message);
        MessageBoxW(NULL, wMessage, L"[OpenGLDebug::MessageBox] OpenGL Critical Error", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
}