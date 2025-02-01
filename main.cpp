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

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

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

    MainWindow mainWindow(hInstance);



    if (mainWindow.Create()) {
        int width = mainWindow.GetWidth();
        int height = mainWindow.GetHeight();
        OpenGLInitializer glInit(mainWindow.GetHwnd());
        if (!glInit.Initialize()) {
            MessageBox(NULL, L"OpenGL Initialization Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
            return 1;
        }
        GameController gameController(150, 100); // Создаем GameController
        gameController.randomizeGrid(0.3f);

        Renderer renderer(width, height);
        renderer.SetGameController(&gameController);
        
        renderer.GetCamera().SetPosition(25.0f, 25.0f, 65.0f); // Камера дальше по Z       

        // Передаем один и тот же экземпляр gameController в WindowController
        WindowController controller(&mainWindow, &renderer, &gameController);
        mainWindow.SetController(&controller);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            //gameController.update();
            renderer.Draw();
        }
    } else {
        MessageBox(NULL, L"Window creation failed", L"Error", MB_OK | MB_ICONERROR);
        std::cout.rdbuf(coutbuf); // восстанавливаем буфер cout
        return 1;
    }
    std::cout.rdbuf(coutbuf); // восстанавливаем буфер cout
    return 0;

}
