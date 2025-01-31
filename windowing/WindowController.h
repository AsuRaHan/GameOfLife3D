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

    bool mouseCaptured; // ���� ��� ������������ ������� ����
    int lastMouseX, lastMouseY; // ��������� ���������� ����

    UINT_PTR timerId; // ������������� �������

    void HandleMouseClick(int screenX, int screenY);
};