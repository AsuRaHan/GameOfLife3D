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

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "Comctl32.lib")



// ������� ��� �������� ��������� ������
void ParseCommandLine(LPWSTR lpCmdLine, int& width, int& height) {
    std::wstring cmdLine(lpCmdLine);
    std::wistringstream iss(cmdLine);
    std::wstring token;

    // ������������� �������� �� ���������
    width = 300;  // �������� �� ��������� ��� ������
    height = 200; // �������� �� ��������� ��� ������

    // ������ ��������� ������
    while (iss >> token) {
        if (token == L"-gridWidth") {
            if (iss >> token) {
                width = std::stoi(token); // ����������� ������ � ����� �����
            }
        }
        else if (token == L"-gridHeight") {
            if (iss >> token) {
                height = std::stoi(token); // ����������� ������ � ����� �����
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
    std::streambuf* coutbuf = std::cout.rdbuf(); // ��������� ����� cout
    std::cout.rdbuf(out.rdbuf()); // �������������� cout � ����

    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);

    std::cout << "LOG! " << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    // ���������� ��� ������ � ������
    int gridWidth, gridHeight;
    ParseCommandLine(lpCmdLine, gridWidth, gridHeight); // �������� �������� �� ��������� ������

    MainWindow mainWindow(hInstance, 800, 600);


    if (mainWindow.Create()) {
        int width = mainWindow.GetWidth();
        int height = mainWindow.GetHeight();
        OpenGLInitializer glInit(mainWindow.GetHwnd());
        if (!glInit.Initialize()) {
            MessageBox(NULL, L"OpenGL Initialization Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
            return 1;
        }
        GameController gameController(gridWidth, gridHeight); // ������� GameController
        gameController.randomizeGrid(0.1f);

        Renderer renderer(width, height);
        renderer.SetGameController(&gameController);

        // �������� ���� � ��� �� ��������� gameController � WindowController
        WindowController controller(&mainWindow, &renderer, &gameController);
        mainWindow.SetController(&controller);

        // �������� ���� ��������
        //SettingsWindow settingsWindow(hInstance);
        //if (!settingsWindow.Create(mainWindow.GetHwnd())) {
        //    MessageBox(NULL, L"Failed to create settings window", L"Error", MB_OK | MB_ICONERROR);
        //    return 1;
        //}
        //settingsWindow.Hide();
        MSG msg;
        bool MainLoop = true;
        // �������� ���������� ���-�� ��� ����� (��������, � ������ ������� ��� ��� ���� ������)
        std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
        while (MainLoop) {
            while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { // PeekMessage(&msg, 0, 0, 0, PM_REMOVE) GetMessage(&msg, NULL, 0, 0)
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT) {
                    MainLoop = false; // ���������� ����������
                }
                //����� ����� �������� ������ ��� ������ ���� ��������
                //if (msg.message == WM_KEYDOWN && msg.wParam == 'P') {
                //    settingsWindow.Show();
                //}
            }
            // ���������� ������� ��� ����������
            std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - lastTime);
            lastTime = currentTime;

            // ����� �������� update � deltaTime
            gameController.update(deltaTime.count()); // .count() ���������� �������� � �������� ��� float
            renderer.Draw();
        }
    }
    else {
        MessageBox(NULL, L"Window creation failed", L"Error", MB_OK | MB_ICONERROR);
        std::cout.rdbuf(coutbuf); // ��������������� ����� cout
        return 1;
    }
    std::cout.rdbuf(coutbuf); // ��������������� ����� cout
    return 0;

}
