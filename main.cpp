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

#pragma comment(lib, "opengl32.lib")


// Функция для парсинга командной строки
void ParseCommandLine(const std::wstring& cmdLine, int& gridWidth, int& gridHeight, int& screenWidth, int& screenHeight, bool& fullscreen) {
    std::wistringstream iss(cmdLine);
    std::wstring token;

    // Устанавливаем значения по умолчанию
    gridWidth = 400;      // Значение по умолчанию для ширины
    gridHeight = 300;     // Значение по умолчанию для высоты
    fullscreen = false; // Значение по умолчанию для полноэкранного режима

    // Парсим командную строку
    while (iss >> token) {
        if (token == L"-gridWidth") {
            if (iss >> token) {
                try {
                    gridWidth = std::stoi(token); // Преобразуем строку в целое число
                    if (gridWidth <= 0) {
                        throw std::invalid_argument("gridWidth must be positive.");
                    }
                }
                catch (const std::exception& e) {
                    std::wcerr << L"Invalid gridWidth value: " << token << L". Using default gridWidth." << std::endl;
                    gridWidth = 400; // Возвращаем значение по умолчанию
                }
            }
        }
        else if (token == L"-gridHeight") {
            if (iss >> token) {
                try {
                    gridHeight = std::stoi(token); // Преобразуем строку в целое число
                    if (gridHeight <= 0) {
                        throw std::invalid_argument("gridHeight must be positive.");
                    }
                }
                catch (const std::exception& e) {
                    std::wcerr << L"Invalid height value: " << token << L". Using default gridHeight." << std::endl;
                    gridHeight = 300; // Возвращаем значение по умолчанию
                }
            }
        }
        else if (token == L"-fullscreen") {
            fullscreen = true; // Если параметр указан, устанавливаем fullscreen в true
        }

        else if (token == L"-screenResolution") { // Новый параметр для разрешения экрана
            if (iss >> token) {
                size_t xPos = token.find(L'x'); // Находим позицию 'x'
                if (xPos != std::wstring::npos) {
                    try {
                        screenWidth = std::stoi(token.substr(0, xPos)); // Получаем ширину
                        screenHeight = std::stoi(token.substr(xPos + 1)); // Получаем высоту
                        if (screenWidth <= 0 || screenHeight <= 0) {
                            throw std::invalid_argument("Width and height must be positive.");
                        }
                    }
                    catch (const std::exception& e) {
                        std::wcerr << L"Invalid screen resolution value: " << token << L". Using default resolution." << std::endl;
                        screenWidth = 1024; // Возвращаем значение по умолчанию
                        screenHeight = 768; // Возвращаем значение по умолчанию
                    }
                }
                else {
                    std::wcerr << L"Invalid screen resolution format: " << token << L". Using default resolution." << std::endl;
                }
            }
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
    io.IniFilename = "gui_setting.ini";
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

    //const std::string& settingsFilename = "settings.ini";
    SettingsManager settings("game_settings.ini");


    // Переменные для ширины и высоты и т.д.
    int gridWidth, gridHeight, windowWidth, windowHeight, windowPosX, windowPosY, screenWidth = 0, screenHeight = 0;
    bool Full;
    if (wcslen(lpCmdLine) != 0) {
        ParseCommandLine(lpCmdLine, gridWidth, gridHeight, screenWidth, screenHeight, Full); // Получаем значения из командной строки
        settings.setSetting("Gameplay", "gridWidth", gridWidth);
        settings.setSetting("Gameplay", "gridHeight", gridHeight);

        settings.setSetting("Graphics", "screenWidth", screenWidth);
        settings.setSetting("Graphics", "screenHeight", screenHeight);
        settings.setSetting("Graphics", "IsFullscreen", Full);
    }
    else {
        gridWidth = settings.getIntSetting("Gameplay", "gridWidth", 400);
        gridHeight = settings.getIntSetting("Gameplay", "gridHeight", 300);

        screenWidth = settings.getIntSetting("Graphics", "screenWidth", 0);
        screenHeight = settings.getIntSetting("Graphics", "screenHeight", 0);
        Full = settings.getBoolSetting("Graphics", "IsFullscreen", false);
    }


    windowWidth = settings.getIntSetting("Graphics", "windowWidth", 800);
    windowHeight = settings.getIntSetting("Graphics", "windowHeight", 600);
    windowPosX = settings.getIntSetting("Graphics", "windowPosX", 0);
    windowPosY = settings.getIntSetting("Graphics", "windowPosY", 0);
    

    MainWindow mainWindow(hInstance, windowWidth, windowHeight, windowPosX, windowPosY);


    if (mainWindow.Create()) {
        OpenGLInitializer glInit(mainWindow.GetHwnd());
        if (!glInit.Initialize(Full, screenWidth, screenHeight)) {
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
        WindowController controller(&renderer, &gameController);
        mainWindow.SetController(&controller);

        initImgui(mainWindow.GetHwnd());
        // ========== кодOхак что бы вызвать обработчики у других классов ====================
        // Вызываем событие WM_SIZE с текущими размерами
        SendMessage(mainWindow.GetHwnd(), WM_SIZE, SIZE_RESTORED, MAKELPARAM(width, height));
        // ===================================================================================
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
                //if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                //    MainLoop = false; // Завершение приложения
                //}
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
    settings.setSetting("Graphics", "windowWidth", mainWindow.GetWidth());
    settings.setSetting("Graphics", "windowHeight", mainWindow.GetHeight());
    settings.setSetting("Graphics", "windowPosX", mainWindow.GetPosX());
    settings.setSetting("Graphics", "windowPosY", mainWindow.GetPosY());
    settings.saveToFile();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    // Восстанавливаем буферы
    std::cout.rdbuf(coutbuf);
    std::cerr.rdbuf(cerrbuf);
    std::clog.rdbuf(clogbuf);
    return 0;

}
