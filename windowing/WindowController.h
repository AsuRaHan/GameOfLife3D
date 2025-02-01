#pragma once
#include <Windows.h>
#include "../rendering/Renderer.h"
#include "../game/GameController.h" 
#include "GridPicker.h"

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
    GridPicker gridPicker;

    bool mouseCaptured; // ���� ��� ������������ ������� ����
    int lastMouseX, lastMouseY; // ��������� ���������� ����
    bool isMiddleButtonDown; // ���� ��� ������������ ������� ������� ������ ����

    UINT_PTR timerId; // ������������� �������
    int simulationSpeedMultiplier = 100; // ��������� �������� (100 ��)
    void UpdateTimer();
    void HandleMouseClick(int screenX, int screenY);

    void RotateCamera(float yaw, float pitch);
    void MoveCamera(float dx, float dy);
    void ResetCamera();

};