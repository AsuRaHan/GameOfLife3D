#pragma once
#include <Windows.h>
#include "../rendering/Renderer.h"
#include "../game/GameController.h" 

class MainWindow; // Forward declaration

class WindowController {
public:
    WindowController(MainWindow* window, Renderer* renderer, GameController* gameController);
    ~WindowController();

    void Resize(int width, int height);
    void HandleEvent(UINT message, WPARAM wParam, LPARAM lParam);

private:
    MainWindow* pWindow;
    Renderer* pRenderer;
    GameController* pGameController;

    bool mouseCaptured; // Флаг для отслеживания захвата мыши
    int lastMouseX, lastMouseY; // Последние координаты мыши

    UINT_PTR timerId; // Идентификатор таймера

    void HandleMouseClick(int screenX, int screenY);
};