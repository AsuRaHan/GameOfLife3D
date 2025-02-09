#pragma once
#ifndef UIRenderer_H_
#define UIRenderer_H_

#include <windows.h>
#include <string>
#include "../res/resource.h"

#include "../game/GameController.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"

class UIRenderer {
private:
    GameController* gameController;
    GPUAutomaton* gpuAutomaton;

    bool simulationWindowVisible = false;
    bool settingsWindowVisible = false;

    bool gameSettingsWindowVisible = false;
    bool saveSettingsWindowVisible = false;
    bool fieldSettingsWindowVisible = false;

    bool aboutWindowVisible = false;
    bool exitDialogVisible = false;

    bool showExitDialog;

    int fieldWidth = 400;  // Пример значений, которые можно изменять
    int fieldHeight = 300;

    char saveFilename[128] = "state.txt";
    std::string aboutText;

    std::string LoadTextFromResource(HINSTANCE hInstance, int resourceId);

    void DrawMenuBar();
    void DrawSimulationWindow();
    void DrawGameSettingsWindow();
    void DrawSaveSettingsWindow();
    void DrawFieldSettingsWindow();
    void DrawAboutWindow();
    void DrawExitDialog();

public:
    UIRenderer(GameController* gc);

    void SetGPUAutomaton(GPUAutomaton* ga) { gpuAutomaton = ga; } // Добавляем сеттер
    void InitializeUI();
    void DrawUI();
    void UpdateUIState();
};

#endif // UIRenderer_H_