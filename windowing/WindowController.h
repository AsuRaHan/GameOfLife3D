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
    int windowWidth; // поле для ширины
    int windowHeight; // поле для высоты

    bool mouseCaptured; // Флаг для отслеживания захвата мыши
    int lastMouseX, lastMouseY; // Последние координаты мыши
    bool isMiddleButtonDown; // Флаг для отслеживания нажатия средней кнопки мыши

    //UINT_PTR timerId; // Идентификатор таймера
    float simulationSpeedMultiplier = 0.01f; // Начальная скорость (100 мс)
    
    bool isLeftButtonDown; // Флаг для отслеживания нажатия средней кнопки мыши
    void StartSelection(int screenX, int screenY); // Новый метод для начала выделения
    void UpdateSelection(int screenX, int screenY); // Новый метод для обновления выделения

    void HandleMouse(int screenX, int screenY, bool isMove = false);
    void PlacePattern(int screenX, int screenY); // метод для размещения шаблона
    void RotateCamera(float yaw, float pitch);
    void MoveCamera(float dx, float dy);
    void ResetCamera();

};
#endif // WINDOWCONTROLLER_H_