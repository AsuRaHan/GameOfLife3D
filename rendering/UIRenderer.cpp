#include "UIRenderer.h"

UIRenderer::UIRenderer(GameController* gc) : gameController(gc), showExitDialog(false) {
    aboutText = LoadTextFromResource(GetModuleHandle(NULL), IDR_ABOUT_TEXT);
    gpuAutomaton = &gc->getGPUAutomaton();
    buttonSize = ImVec2(200, 30);
}

void UIRenderer::InitializeUI() {
    // Вы можете добавить здесь инициализацию или настройку UI, если это необходимо
}

std::string UIRenderer::LoadTextFromResource(HINSTANCE hInstance, int resourceId) {
    HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!hResource) {
        // Обработка ошибки
        return "";
    }

    HGLOBAL hResourceData = LoadResource(hInstance, hResource);
    if (!hResourceData) {
        // Обработка ошибки
        return "";
    }

    DWORD resourceSize = SizeofResource(hInstance, hResource);
    const char* resourceData = (const char*)LockResource(hResourceData);

    // Копируем данные в строку
    std::string text(resourceData, resourceSize);

    return text;
}

void UIRenderer::DrawUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    DrawMenuBar();
    DrawSimulationWindow();
    DrawGameSettingsWindow();
    DrawSaveSettingsWindow();
    DrawFieldSettingsWindow();
    DrawAboutWindow();
    DrawExitDialog();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void UIRenderer::DrawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::MenuItem("Выход")) {
            exitDialogVisible = true;
        }
        if (ImGui::BeginMenu("Окна")) {
            ImGui::MenuItem("Управление симуляцией", NULL, &simulationWindowVisible);
            ImGui::MenuItem("Настройки игры", NULL, &gameSettingsWindowVisible);
            ImGui::MenuItem("Настройки сохранения", NULL, &saveSettingsWindowVisible);
            ImGui::MenuItem("Настройки поля", NULL, &fieldSettingsWindowVisible);
            ImGui::MenuItem("О программе", NULL, &aboutWindowVisible);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void UIRenderer::DrawSimulationWindow() {
    if (!simulationWindowVisible) return;

    ImGui::Begin("Симуляция", &simulationWindowVisible, ImGuiWindowFlags_NoResize);
    //ImGui::SetWindowPos(ImVec2(2, 2), ImGuiCond_Once);
    //ImGui::SetWindowSize(ImVec2(320, 0), ImGuiCond_Once);

    // Управление симуляцией
    if (gameController->isSimulationRunning()) {
        if (ImGui::Button("Остановить симуляцию", buttonSize)) {
            gameController->stopSimulation();
        }
    }
    else {
        if (ImGui::Button("Начать симуляцию", buttonSize)) {
            gameController->startSimulation();
        }
    }
    if (ImGui::Button("Шаг симуляции", buttonSize)) {
        gameController->stepSimulation();
    }
    if (ImGui::Button("Очистить поле", buttonSize)) {
        gameController->clearGrid();
    }
    if (ImGui::Button("Случайные клетки", buttonSize)) {
        gameController->randomizeGrid(0.1f);
    }
    ImGui::Separator();
    // Выбор фигуры и поворот
    ImGui::Text("Фигура для добавления");
    static int selectedPattern = 0;
    const char* patterns[] = { "Glider", "Blinker", "Toad", "Beacon", "Pentadecathlon", "Gosper Glider Gun" };
    if (ImGui::Combo("##selectPattern", &selectedPattern, patterns, IM_ARRAYSIZE(patterns))) {
        gameController->setCurrentPattern(selectedPattern + 1);
    }
    ImGui::Separator();
    ImGui::Text("Вращение фигуры");
    static int selectedPatternRotator = 0;
    const char* patternsRotator[] = { "None", "Rotate 90", "Rotate 180", "Rotate 270", "Flip Horizontal", "Flip Vertical" };
    if (ImGui::Combo("##selectRotator", &selectedPatternRotator, patternsRotator, IM_ARRAYSIZE(patternsRotator))) {
        gameController->setCurrentPatternRotator(selectedPatternRotator);
    }

    ImGui::End();
}

void UIRenderer::DrawGameSettingsWindow() {
    if (!gameSettingsWindowVisible) return;

    ImGui::Begin("Игра", &gameSettingsWindowVisible, ImGuiWindowFlags_NoResize);
    //ImGui::SetWindowSize(ImVec2(300, 0), ImGuiCond_Once);

    if (gpuAutomaton) {
        ImGui::Text("Правила игры:");
        ImGui::Separator();
        ImGui::Text("Рождение");
        int selectedBirth = gpuAutomaton->birth - 1;
        const char* birthOptions[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
        if (ImGui::Combo("##birth", &selectedBirth, birthOptions, IM_ARRAYSIZE(birthOptions))) {
            gpuAutomaton->birth = selectedBirth + 1;
        }
        ImGui::Separator();
        ImGui::Text("Выживание (мин)");
        const char* survivalMinOptions[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
        ImGui::Combo("##survivalMin", &gpuAutomaton->survivalMin, survivalMinOptions, IM_ARRAYSIZE(survivalMinOptions));
        ImGui::Separator();
        ImGui::Text("Выживание (макс)");
        const char* survivalMaxOptions[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
        ImGui::Combo("##survivalMax", &gpuAutomaton->survivalMax, survivalMaxOptions, IM_ARRAYSIZE(survivalMaxOptions));
        ImGui::Separator();
        ImGui::Text("Перенаселение");
        int selectedOverPopulation = gpuAutomaton->overpopulation - 1;
        const char* overpopulationOptions[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
        if (ImGui::Combo("##overpopulation", &selectedOverPopulation, overpopulationOptions, IM_ARRAYSIZE(overpopulationOptions))) {
            gpuAutomaton->overpopulation = selectedOverPopulation + 1;
        }
    }
    else {
        ImGui::Text("GPUAutomaton не инициализирован!");
    }

    ImGui::End();
}

void UIRenderer::DrawSaveSettingsWindow() {
    if (!saveSettingsWindowVisible) return;

    ImGui::Begin("Сохранения", &saveSettingsWindowVisible, ImGuiWindowFlags_NoResize);
    //ImGui::SetWindowSize(ImVec2(300, 0), ImGuiCond_Once);

    ImGui::Text("Имя файла");
    ImGui::InputText("##fileName", saveFilename, IM_ARRAYSIZE(saveFilename));
    
    if (ImGui::Button("Сохранить", buttonSize)) {
        gameController->saveGameState(saveFilename);
    }
    if (ImGui::Button("Загрузить", buttonSize)) {
        gameController->loadGameState(saveFilename);
    }

    ImGui::End();
}

void UIRenderer::DrawFieldSettingsWindow() {
    if (!fieldSettingsWindowVisible) return;

    ImGui::Begin("Настройки поля", &fieldSettingsWindowVisible, ImGuiWindowFlags_NoResize);
    //ImGui::SetWindowSize(ImVec2(150, 0), ImGuiCond_Once);

    ImGui::InputInt("Ширина", &fieldWidth);
    ImGui::InputInt("Высота", &fieldHeight);

    if (ImGui::Button("Применить", buttonSize)) {
        // Здесь нужно вызвать метод для изменения размеров поля в вашем gameController
        gameController->setFieldSize(fieldWidth, fieldHeight);
    }

    ImGui::End();
}

void UIRenderer::DrawAboutWindow() {
    if (!aboutWindowVisible) return;
    ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing); // Ширина 500 пикселей, высота автоматическая
    ImGui::Begin("О программе", &aboutWindowVisible, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextWrapped("%s", aboutText.c_str());
    if (ImGui::Button("Закрыть", buttonSize)) {
        aboutWindowVisible = false;
    }
    ImGui::End();
}

void UIRenderer::DrawExitDialog() {
    if (!exitDialogVisible) return;

    ImGui::OpenPopup("Вы уверены?");
    if (ImGui::BeginPopupModal("Вы уверены?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Вы действительно хотите выйти?\n");
        if (ImGui::Button("Да", ImVec2(120, 0))) {
            PostQuitMessage(0);
            exitDialogVisible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Нет", ImVec2(120, 0))) {
            exitDialogVisible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void UIRenderer::UpdateUIState() {
    // Это место для обновления состояния UI на основе изменений в игре или других событий
}