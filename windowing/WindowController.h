#pragma once

#ifndef WINDOWCONTROLLER_H_
#define WINDOWCONTROLLER_H_

#include "../rendering/IRendererProvider.h"
#include "../game/GameController.h" 
#include "../system/InputHandler.h"
#include "../rendering/CameraController.h"
#include "GridPicker.h"

class MainWindow; // Forward declaration

class WindowController {
public:
    WindowController(MainWindow* window, IRendererProvider* renderer, GameController* gameController);
    ~WindowController();

    void Resize(int width, int height);
    void HandleEvent(UINT message, WPARAM wParam, LPARAM lParam);

private:
    MainWindow* pWindow;
    IRendererProvider* pRenderer;
    GameController* pGameController;
    GridPicker gridPicker;

    InputHandler inputHandler;
    CameraController pCameraController; 


    bool mouseCaptured; // ���� ��� ������������ ������� ����
    int lastMouseX, lastMouseY; // ��������� ���������� ����
    bool isMiddleButtonDown; // ���� ��� ������������ ������� ������� ������ ����

    //UINT_PTR timerId; // ������������� �������
    float simulationSpeedMultiplier = 0.01f; // ��������� �������� (100 ��)
    //void UpdateTimer();
    void HandleMouseClick(int screenX, int screenY);
    void PlacePattern(int screenX, int screenY); // ����� ��� ���������� �������
    void RotateCamera(float yaw, float pitch);
    void MoveCamera(float dx, float dy);
    void ResetCamera();

};
#endif // WINDOWCONTROLLER_H_