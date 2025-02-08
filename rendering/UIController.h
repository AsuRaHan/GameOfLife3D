#pragma once
#ifndef UICONTROLLER_H_
#define UICONTROLLER_H_

#include <windows.h>
#include <string>
#include "../res/resource.h"

#include "../game/GameController.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"

class UIController {
private:
    GameController* gameController;
    GPUAutomaton* gpuAutomaton;

    bool showExitDialog;
    std::string aboutText;

public:
    UIController(GameController* gc);
    void SetGPUAutomaton(GPUAutomaton* ga) { gpuAutomaton = ga; } // Добавляем сеттер

    void InitializeUI();
    std::string LoadTextFromResource(HINSTANCE hInstance, int resourceId);
    void DrawUI();
    void DrawMenuBar();
    void UpdateUIState();
};

#endif // UICONTROLLER_H_