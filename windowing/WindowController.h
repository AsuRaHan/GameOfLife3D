#pragma once
#ifndef WINDOWCONTROLLER_H_
#define WINDOWCONTROLLER_H_

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

    bool mouseCaptured; // Флаг для отслеживания захвата мыши
    int lastMouseX, lastMouseY; // Последние координаты мыши
    bool isMiddleButtonDown; // Флаг для отслеживания нажатия средней кнопки мыши

    //UINT_PTR timerId; // Идентификатор таймера
    float simulationSpeedMultiplier = 0.01f; // Начальная скорость (100 мс)
    //void UpdateTimer();
    void HandleMouseClick(int screenX, int screenY);
    void PlacePattern(int screenX, int screenY); // метод для размещения шаблона
    void RotateCamera(float yaw, float pitch);
    void MoveCamera(float dx, float dy);
    void ResetCamera();

};
#endif // WINDOWCONTROLLER_H_