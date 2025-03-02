#include "windowing/MainWindow.h"
#include "windowing/WindowController.h"
#include "rendering/Renderer.h"
#include "game/GameController.h"
#include "system/OpenGLInitializer.h"
#include "system/SettingsManager.h"
#include "system/PerformanceStats.h"
#include "system/CommandLineParser.h"

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

int GetMonitorRefreshRate() {
    DEVMODE dm = { 0 };
    dm.dmSize = sizeof(DEVMODE);
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm)) {
        return dm.dmDisplayFrequency; // Частота в Гц (например, 75)
    }
    return 60; // Значение по умолчанию, если не удалось получить
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

    SettingsManager settings("game_settings.ini");

    CommandLineParser parser(lpCmdLine);
    CommandLineParser::Options cmdOptions = parser.parse();
    // Use the parsed options
    int gridWidth = cmdOptions.gridWidth;
    int gridHeight = cmdOptions.gridHeight;
    bool fullScreen = cmdOptions.fullscreen;
    int screenWidth = cmdOptions.screenWidth;
    int screenHeight = cmdOptions.screenHeight;

    if (wcslen(lpCmdLine) != 0) {

        settings.setSetting("Gameplay", "gridWidth", gridWidth);
        settings.setSetting("Gameplay", "gridHeight", gridHeight);

        settings.setSetting("Graphics", "screenWidth", screenWidth);
        settings.setSetting("Graphics", "screenHeight", screenHeight);
        settings.setSetting("Graphics", "IsFullscreen", fullScreen);
    }
    else {
        gridWidth = settings.getIntSetting("Gameplay", "gridWidth", 400);
        gridHeight = settings.getIntSetting("Gameplay", "gridHeight", 300);

        screenWidth = settings.getIntSetting("Graphics", "screenWidth", 0);
        screenHeight = settings.getIntSetting("Graphics", "screenHeight", 0);
        fullScreen = settings.getBoolSetting("Graphics", "IsFullscreen", false);
    }


    int windowWidth = settings.getIntSetting("Graphics", "windowWidth", 1024);
    int windowHeight = settings.getIntSetting("Graphics", "windowHeight", 768);
    int windowPosX = settings.getIntSetting("Graphics", "windowPosX", 0);
    int windowPosY = settings.getIntSetting("Graphics", "windowPosY", 0);
    

    MainWindow mainWindow(hInstance, windowWidth, windowHeight, windowPosX, windowPosY);
    HWND mHnd = mainWindow.Create();
    if (mHnd) {
        OpenGLInitializer glInit(mHnd);
        if (!glInit.Initialize(fullScreen, screenWidth, screenHeight)) {
            MessageBox(NULL, L"OpenGL Initialization Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
            return 1;
        }

        int width = mainWindow.GetWidth();
        int height = mainWindow.GetHeight();

        GameController gameController(gridWidth, gridHeight); // Создаем GameController и задаем размер игрового поля
        gameController.randomizeGrid(0.05f); // заполняем игровое поле случайными живыми клетками

        Renderer renderer(width, height);
        renderer.SetGameController(&gameController);
        gameController.SetRendererProvider(&renderer);

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
        // так как у нас есть вертикальная синхронизация то колличество симуляций не сможет превысить частоту обновление монитора. поэтому мы обошли такие ограничения
        PerformanceStats& stats = PerformanceStats::getInstance();

        int monitorHz = GetMonitorRefreshRate();
        float frameIntervalMs = 1000.0f / monitorHz; // Например, 13.33 мс для 75 Гц
        stats.setTargetRefreshRate(monitorHz);
        auto lastFrameTime = std::chrono::steady_clock::now();
        float targetFPS = static_cast<float>(monitorHz); // Целевая частота монитора
        float simulationSteps = 1.0f; // Теперь float для плавности
        int needSteps = 1;

        static float smoothedSimulationSteps = 1.0f; // Добавляем для плавности, вне цикла

        while (MainLoop) {
            while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { 
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT) {
                    MainLoop = false; // Завершение приложения
                }
            }
            if (gameController.getTurboBoost()) { // экстперементальный турбо режим. работает не стабильно может повесить ОС
                auto now = std::chrono::steady_clock::now();
                auto elapsedFrameNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastFrameTime).count();
                float elapsedFrameMs = elapsedFrameNs / 1000000.0f;

                // Обновляем статистику один раз за кадр
                stats.updateStats();
                float currentFPS = stats.getFPS();

                // Плавная регулировка числа шагов симуляции
                float fpsError = currentFPS - targetFPS; // Больше FPS -> больше шагов

                // Мёртвая зона: игнорируем мелкие отклонения (±5)
                float deadZone = 9.0f;
                float adjustment = 0.0f;
                if (fpsError > deadZone) {
                    adjustment = (fpsError - deadZone) * 0.1f; // Рост при высоком FPS
                }
                else if (fpsError < -deadZone) {
                    adjustment = (fpsError + deadZone) * 0.1f; // Снижение при просадке
                }
                else if (fpsError >= 0.0f) {
                    adjustment = 0.2f; // Минимальный прирост, если FPS не ниже целевого
                }

                // Плавное обновление через сглаживание
                float smoothingFactor = 0.1f; // Плавность изменений
                smoothedSimulationSteps = smoothedSimulationSteps + smoothingFactor * (simulationSteps + adjustment - smoothedSimulationSteps);
                simulationSteps = smoothedSimulationSteps;

                // Жёсткий сброс при сильной просадке
                if (currentFPS < targetFPS * 0.2f) {
                    simulationSteps -= 1.0f;
                    smoothedSimulationSteps -= 1.0f;
                }

                // Ограничиваем диапазон без std::min и std::max
                if (simulationSteps < 1.0f) {
                    simulationSteps = 1.0f;
                    smoothedSimulationSteps = 1.0f;
                }
                if (simulationSteps > 25.0f) {
                    simulationSteps = 25;
                    smoothedSimulationSteps = 25.0f;
                }
                needSteps = static_cast<int>(simulationSteps + 0.5f);
                //gameController.update();
                if (elapsedFrameMs >= frameIntervalMs) {
                    for (int i = 0; i < needSteps; i++) {
                        gameController.update();
                    }
                    renderer.Draw();
                    stats.recordFrame();
                    lastFrameTime = now;
                }
               
            }
            else {
                needSteps = 1;
                PerformanceStats::getInstance().updateStats();
                gameController.update();
                renderer.Draw();
                PerformanceStats::getInstance().recordFrame();
            }


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