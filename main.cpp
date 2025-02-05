#include "windowing/MainWindow.h"
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
#include "imgui_impl_win32.h" // ���� �� ��������� ��� Windows

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


// ������� ��� �������� ��������� ������
void ParseCommandLine(LPWSTR lpCmdLine, int& width, int& height) {
    std::wstring cmdLine(lpCmdLine);
    std::wistringstream iss(cmdLine);
    std::wstring token;

    // ������������� �������� �� ���������
    width = 400;  // �������� �� ��������� ��� ������
    height = 300; // �������� �� ��������� ��� ������

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

void loadFontFromRes(ImGuiIO io) {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_FONT_CUSTOM), RT_RCDATA);
    if (hRes == NULL) {
        DWORD errorCode = GetLastError();
        // ��������� ������, ��������:
        std::cout << "������ ��� ������ �������: " << std::hex << errorCode << std::endl;
    }
    HGLOBAL hGlob = LoadResource(NULL, hRes);
    DWORD dwSize = SizeofResource(NULL, hRes);
    const void* pData = LockResource(hGlob);

    if (pData) {
        unsigned char* pFontData = new unsigned char[dwSize];
        memcpy(pFontData, pData, dwSize);

        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false; // �� ��������� �������
        ImFont* font = io.Fonts->AddFontFromMemoryTTF(pFontData, static_cast<int>(dwSize), 16.0f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

        if (font) {
            ImGui::GetIO().FontDefault = font;
        }
        else {
            std::cout << "�� ������� ��������� ����� �� ��������!" << std::endl;
            delete[] pFontData; // ����������� ������, ���� ����� �� ����������
        }
    }
    else {
        std::cout << "������ ������ �� ������!" << std::endl;
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

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
        io.IniFilename = NULL; // ��������� ���������� � ini ����
        loadFontFromRes(io);
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_InitForOpenGL(mainWindow.GetHwnd());
        ImGui_ImplOpenGL3_Init();

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
                if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                    MainLoop = false; // ���������� ����������
                }
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    std::cout.rdbuf(coutbuf); // ��������������� ����� cout
    return 0;

}
