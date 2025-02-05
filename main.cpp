#include "windowing/MainWindow.h"
#include "windowing/SettingsWindow.h"
#include "windowing/WindowController.h"
#include "rendering/Renderer.h"
#include "game/GameController.h"
#include "system/OpenGLInitializer.h"

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
#pragma comment(lib, "Comctl32.lib")




// Функция для парсинга командной строки
void ParseCommandLine(LPWSTR lpCmdLine, int& width, int& height) {
    std::wstring cmdLine(lpCmdLine);
    std::wistringstream iss(cmdLine);
    std::wstring token;

    // Устанавливаем значения по умолчанию
    width = 300;  // Значение по умолчанию для ширины
    height = 200; // Значение по умолчанию для высоты

    // Парсим командную строку
    while (iss >> token) {
        if (token == L"-gridWidth") {
            if (iss >> token) {
                width = std::stoi(token); // Преобразуем строку в целое число
            }
        }
        else if (token == L"-gridHeight") {
            if (iss >> token) {
                height = std::stoi(token); // Преобразуем строку в целое число
            }
        }
    }
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
    std::cout.rdbuf(out.rdbuf()); // перенаправляем cout в файл

    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);

    std::cout << "LOG! " << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    // Переменные для ширины и высоты
    int gridWidth, gridHeight;
    ParseCommandLine(lpCmdLine, gridWidth, gridHeight); // Получаем значения из командной строки

    MainWindow mainWindow(hInstance, 800, 600);


    if (mainWindow.Create()) {
        int width = mainWindow.GetWidth();
        int height = mainWindow.GetHeight();
        OpenGLInitializer glInit(mainWindow.GetHwnd());
        if (!glInit.Initialize()) {
            MessageBox(NULL, L"OpenGL Initialization Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
            return 1;
        }

        GameController gameController(gridWidth, gridHeight); // Создаем GameController
        gameController.randomizeGrid(0.1f);

        Renderer renderer(width, height);
        renderer.SetGameController(&gameController);

        // Передаем один и тот же экземпляр gameController в WindowController
        WindowController controller(&mainWindow, &renderer, &gameController);
        mainWindow.SetController(&controller);

        // Создание окна настроек
        //SettingsWindow settingsWindow(hInstance);
        //if (!settingsWindow.Create(mainWindow.GetHwnd())) {
        //    MessageBox(NULL, L"Failed to create settings window", L"Error", MB_OK | MB_ICONERROR);
        //    return 1;
        //}

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_InitForOpenGL(mainWindow.GetHwnd());
        ImGui_ImplOpenGL3_Init();

        MSG msg;
        bool MainLoop = true;
        // Объявите переменные где-то вне цикла (например, в начале функции или как член класса)
        std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
        while (MainLoop) {
            while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { // PeekMessage(&msg, 0, 0, 0, PM_REMOVE) GetMessage(&msg, NULL, 0, 0)
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT) {
                    MainLoop = false; // Завершение приложения
                }
                //Здесь можно добавить логику для показа окна настроек
                //if (msg.message == WM_KEYDOWN && msg.wParam == 'P') {
                //    settingsWindow.Show();
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
        std::cout.rdbuf(coutbuf); // восстанавливаем буфер cout
        return 1;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    std::cout.rdbuf(coutbuf); // восстанавливаем буфер cout
    return 0;

}
