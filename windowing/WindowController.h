#pragma once

#ifndef WINDOWCONTROLLER_H_
#define WINDOWCONTROLLER_H_

#include "../rendering/IRendererProvider.h"
#include "../game/GameController.h" 
#include "../system/InputHandler.h"
#include "../rendering/CameraController.h"
#include "GridPicker.h"

class WindowController {
public:
    WindowController(IRendererProvider* renderer, GameController* gameController);
    ~WindowController();

    void Resize(int width, int height);
    void HandleEvent(UINT message, WPARAM wParam, LPARAM lParam);

private:
    IRendererProvider* pRenderer;
    GameController* pGameController;
    GridPicker gridPicker;

    InputHandler inputHandler;
    CameraController pCameraController; 
    int windowWidth; // ���� ��� ������
    int windowHeight; // ���� ��� ������

    bool mouseCaptured; // ���� ��� ������������ ������� ����
    int lastMouseX, lastMouseY; // ��������� ���������� ����
    bool isMiddleButtonDown; // ���� ��� ������������ ������� ������� ������ ����

    //UINT_PTR timerId; // ������������� �������
    float simulationSpeedMultiplier = 0.01f; // ��������� �������� (100 ��)
    
    bool isLeftButtonDown; // ���� ��� ������������ ������� ������� ������ ����
    void StartSelection(int screenX, int screenY); // ����� ����� ��� ������ ���������
    void UpdateSelection(int screenX, int screenY); // ����� ����� ��� ���������� ���������

    void HandleMouse(int screenX, int screenY, bool isMove = false);
    void PlacePattern(int screenX, int screenY); // ����� ��� ���������� �������
    void RotateCamera(float yaw, float pitch);
    void MoveCamera(float dx, float dy);
    void ResetCamera();

};
#endif // WINDOWCONTROLLER_H_