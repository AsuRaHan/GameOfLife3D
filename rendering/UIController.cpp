#include "UIController.h"

UIController::UIController(GameController* gc) : gameController(gc), showExitDialog(false) {}

void UIController::InitializeUI() {
    // Вы можете добавить здесь инициализацию или настройку UI, если это необходимо
}

void UIController::DrawUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    // ------------------------------------ главное меню игры --------------------------------
    ImGui::Begin("Управление игрой", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::SetWindowPos(ImVec2(5, 5), ImGuiCond_Once);
    //ImGui::SetWindowSize(ImVec2(170, 150), ImGuiCond_Once);

    // Находим максимальную ширину текста всех кнопок
    ImVec2 maxSize(0, 0);
    const char* buttons[] = { 
        "Начать симуляцию", 
        "Остановить симуляцию", 
        "Шаг симуляции", 
        //"Предыдущее поколение", 
        "Очистить поле", 
        "Случайные клетки",
        "Выход"
    };
    for (int i = 0; i < IM_ARRAYSIZE(buttons); ++i) {
        ImVec2 size = ImGui::CalcTextSize(buttons[i]);
        if (size.x > maxSize.x) {
            maxSize = size;
        }
    }
    // Добавляем некоторый запас к ширине для лучшей визуализации
    float buttonWidth = maxSize.x + 20; // 20 - это запас по ширине

    if (ImGui::Button("Начать симуляцию", ImVec2(buttonWidth, 0))) {
        gameController->startSimulation();
    }
    if (ImGui::Button("Остановить симуляцию", ImVec2(buttonWidth, 0))) {
        gameController->stopSimulation();
    }
    if (ImGui::Button("Шаг симуляции", ImVec2(buttonWidth, 0))) {
        gameController->stepSimulation();
    }
    //if (ImGui::Button("Предыдущее поколение", ImVec2(buttonWidth, 0))) {
    //    gameController->previousGeneration();
    //}
    if (ImGui::Button("Очистить поле", ImVec2(buttonWidth, 0))) {
        gameController->clearGrid();
    }
    if (ImGui::Button("Случайные клетки", ImVec2(buttonWidth, 0))) {
        gameController->randomizeGrid(0.1f);
    }
    //// Выпадающий список (ComboBox)
    static int selectedPattern = 0;
    const char* patterns[] = { "Glider", "Blinker", "Toad", "Beacon", "Pentadecathlon","gosperGliderGun","gosperGliderGunFlipped","gosperGliderGunVertical","gosperGliderGunVerticalFlipped"};
    if (ImGui::Combo("Pattern", &selectedPattern, patterns, IM_ARRAYSIZE(patterns))) {
        // Здесь вы можете обработать выбор паттерна, например:
        gameController->setCurrentPattern(selectedPattern + 1); // +1 если нумерация начинается с 1
    }
    if (ImGui::Button("Выход", ImVec2(buttonWidth, 0))) {
        showExitDialog = true;
    }
    // Рендеринг диалога выхода
    if (showExitDialog) {
        ImGui::OpenPopup("Вы уверены?");
        if (ImGui::BeginPopupModal("Вы уверены?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Вы действительно хотите выйти?\n");

            if (ImGui::Button("Да", ImVec2(120, 0))) {
                PostQuitMessage(0);
                showExitDialog = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Нет", ImVec2(120, 0))) {
                showExitDialog = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
    // --------------------------------- конец главного меню игры -----------------------------
    
    // Вторая панель с дополнительными компонентами
    //ImGui::Begin("Additional Controls", NULL, ImGuiWindowFlags_NoResize);
    //ImGui::SetWindowPos(ImVec2(5, 250), ImGuiCond_Once);
    //ImGui::SetWindowSize(ImVec2(150, 150), ImGuiCond_Once);
    //// Текстовое поле
    //static char textBuffer[128] = "Enter text here";
    //ImGui::InputText("Text Input", textBuffer, IM_ARRAYSIZE(textBuffer));

    //// Слайдер
    //static float floatValue = 0.5f;
    //ImGui::SliderFloat("Float Slider", &floatValue, 0.0f, 1.0f, "%.2f");

    //// Добавим кнопку для переключения видимости сетки
    //if (ImGui::Checkbox("Show Grid", &showGrid)) {
    //    ToggleGridVisibility();
    //}
    //ImGui::End();


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIController::UpdateUIState() {
    // Это место для обновления состояния UI на основе изменений в игре или других событий
}