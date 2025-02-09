#include "windowing/MainWindow.h"
#include "windowing/WindowController.h"
#include "rendering/Renderer.h"
#include "game/GameController.h"
#include "system/OpenGLInitializer.h"
#include "system/SettingsManager.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h" // Если вы работаете под Windows

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


// Функция для парсинга командной строки
void ParseCommandLine(const std::wstring& cmdLine, int& width, int& height, bool& fullscreen) {
    std::wistringstream iss(cmdLine);
    std::wstring token;

    // Устанавливаем значения по умолчанию
    width = 400;      // Значение по умолчанию для ширины
    height = 300;     // Значение по умолчанию для высоты
    fullscreen = false; // Значение по умолчанию для полноэкранного режима

    // Парсим командную строку
    while (iss >> token) {
        if (token == L"-gridWidth") {
            if (iss >> token) {
                try {
                    width = std::stoi(token); // Преобразуем строку в целое число
                    if (width <= 0) {
                        throw std::invalid_argument("Width must be positive.");
                    }
                }
                catch (const std::exception& e) {
                    std::wcerr << L"Invalid width value: " << token << L". Using default width." << std::endl;
                    width = 400; // Возвращаем значение по умолчанию
                }
            }
        }
        else if (token == L"-gridHeight") {
            if (iss >> token) {
                try {
                    height = std::stoi(token); // Преобразуем строку в целое число
                    if (height <= 0) {
                        throw std::invalid_argument("Height must be positive.");
                    }
                }
                catch (const std::exception& e) {
                    std::wcerr << L"Invalid height value: " << token << L". Using default height." << std::endl;
                    height = 300; // Возвращаем значение по умолчанию
                }
            }
        }
        else if (token == L"-fullscreen") {
            fullscreen = true; // Если параметр указан, устанавливаем fullscreen в true
        }
    }
}

void loadFontFromRes(ImGuiIO io) {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_FONT_CUSTOM), RT_RCDATA);
    if (hRes == NULL) {
        DWORD errorCode = GetLastError();
        // Обработка ошибки, например:
        std::cout << "Ошибка при поиске ресурса: " << std::hex << errorCode << std::endl;
    }
    HGLOBAL hGlob = LoadResource(NULL, hRes);
    DWORD dwSize = SizeofResource(NULL, hRes);
    const void* pData = LockResource(hGlob);

    if (pData) {
        unsigned char* pFontData = new unsigned char[dwSize];
        memcpy(pFontData, pData, dwSize);

        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false; // Мы управляем памятью
        ImFont* font = io.Fonts->AddFontFromMemoryTTF(pFontData, static_cast<int>(dwSize), 16.0f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

        if (font) {
            ImGui::GetIO().FontDefault = font;
        }
        else {
            std::cout << "Не удалось загрузить шрифт из ресурсов!" << std::endl;
            delete[] pFontData; // Освобождаем память, если шрифт не загрузился
        }
    }
    else {
        std::cout << "Ресурс шрифта не найден!" << std::endl;
    }
}

void initImgui(HWND hwnd) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    io.IniFilename = NULL; // Отключаем сохранение в ini файл
    loadFontFromRes(io);

    ImGuiStyle& style = ImGui::GetStyle();

    // Установим светлую цветовую схему для кнопок
    style.Colors[ImGuiCol_Button] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Светло-серый цвет кнопок
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); // Цвет при наведении
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Цвет при активации

    // Черный текст для кнопок
    style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Применим черный текст и светлые цвета для других элементов
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f); // Фон окна
    style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Фон рамок (слайдеры, текстовые поля)
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Заголовки
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    ImGui::StyleColorsLight(&style);
    // Это нужно вызывать один раз при инициализации вашего приложения или в начале каждого кадра, если нужно динамически менять стили
    
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL3_Init();
}
int wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    std::ofstream out("log.txt");
    std::streambuf* coutbuf = std::cout.rdbuf(); // сохраняем буфер cout
    std::streambuf* cerrbuf = std::cerr.rdbuf(); // сохраняем буфер cerr
    std::streambuf* clogbuf = std::clog.rdbuf(); // сохраняем буфер clog
    std::cout.rdbuf(out.rdbuf()); // перенаправляем cout в файл
    std::cerr.rdbuf(out.rdbuf()); // перенаправляем cerr в тот же файл
    std::clog.rdbuf(out.rdbuf()); // перенаправляем clog в файл

    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);

    std::cout << "LOG! " << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    SettingsManager settingsManager;
    // Переменные для ширины и высоты
    int gridWidth, gridHeight, windowWidth, windowHeight;
    bool Full;
    if (wcslen(lpCmdLine) != 0) {
        ParseCommandLine(lpCmdLine, gridWidth, gridHeight, Full); // Получаем значения из командной строки
        settingsManager.setSetting("gridWidth", gridWidth);
        settingsManager.setSetting("gridHeight", gridHeight);
        settingsManager.setSetting("IsFullscreen", Full);
    }
    else {
        gridWidth = settingsManager.getIntSetting("gridWidth", 400);
        gridHeight = settingsManager.getIntSetting("gridHeight", 300);
        //float volume = settingsManager.getFloatSetting("Volume");
        Full = settingsManager.getBoolSetting("IsFullscreen",false);
    }


    windowWidth = settingsManager.getIntSetting("windowWidth", 800);
    windowHeight = settingsManager.getIntSetting("windowHeight", 600);
    

    MainWindow mainWindow(hInstance, windowWidth, windowHeight);


    if (mainWindow.Create()) {
        OpenGLInitializer glInit(mainWindow.GetHwnd());
        if (!glInit.Initialize(Full)) {
            MessageBox(NULL, L"OpenGL Initialization Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
            return 1;
        }

        int width = mainWindow.GetWidth();
        int height = mainWindow.GetHeight();

        GameController gameController(gridWidth, gridHeight); // Создаем GameController и задаем размер игрового поля
        gameController.randomizeGrid(0.1f); // заполняем игровое поле случайными живыми клетками

        Renderer renderer(width, height);
        renderer.SetGameController(&gameController);
        gameController.SetCellInstanceProvider(&renderer);

        // Передаем один и тот же экземпляр gameController в WindowController
        WindowController controller(&mainWindow, &renderer, &gameController);
        mainWindow.SetController(&controller);

        initImgui(mainWindow.GetHwnd());

        MSG msg;
        bool MainLoop = true;
        std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
        while (MainLoop) {
            while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { // PeekMessage(&msg, 0, 0, 0, PM_REMOVE) GetMessage(&msg, NULL, 0, 0)
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT) {
                    MainLoop = false; // Завершение приложения
                }
                if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                    MainLoop = false; // Завершение приложения
                }
            }

            // Вычисление времени для обновления
            std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - lastTime);
            lastTime = currentTime;

            // Здесь вызываем update с deltaTime
            gameController.update(deltaTime.count()); // .count() возвращает значение в секундах как float
            
            renderer.Draw();
        }

    }
    else {
        MessageBox(NULL, L"Window creation failed", L"Error", MB_OK | MB_ICONERROR);
        // Восстанавливаем буферы
        std::cout.rdbuf(coutbuf);
        std::cerr.rdbuf(cerrbuf);
        std::clog.rdbuf(clogbuf);
        return 1;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    settingsManager.setSetting("windowWidth", mainWindow.GetWidth());
    settingsManager.setSetting("windowHeight", mainWindow.GetHeight());

    // Восстанавливаем буферы
    std::cout.rdbuf(coutbuf);
    std::cerr.rdbuf(cerrbuf);
    std::clog.rdbuf(clogbuf);
    return 0;

}
