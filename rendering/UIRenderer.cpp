#include "UIRenderer.h"

UIRenderer::UIRenderer(GameController* gc) : gameController(gc), showExitDialog(false) {
    aboutText = LoadTextFromResource(GetModuleHandle(NULL), IDR_ABOUT_TEXT);
    gpuAutomaton = &gc->getGPUAutomaton();
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

//void UIRenderer::DrawUI2() {
//    ImGui_ImplOpenGL3_NewFrame();
//    ImGui_ImplWin32_NewFrame();
//    ImGui::NewFrame();
//    // ------------------------------------ главное меню игры --------------------------------
//    bool windowWasHovered = false;
//    ImGui::SetNextWindowSize(ImVec2(0.0f, 650.0f), ImGuiCond_Always);
//    ImGui::Begin("Управление игрой", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
//    // Проверяем, был ли окно под курсором мыши в прошлый раз
//    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow)) {
//        windowWasHovered = true;
//    }
//    else if (windowWasHovered) {
//        // Если мышь только что покинула область окна, снимаем фокус
//        ImGui::SetWindowFocus(NULL); // NULL означает дефокусировку всех окон
//        windowWasHovered = false;
//    }
//    ImGui::SetWindowPos(ImVec2(2, 2), ImGuiCond_Once);
//
//    // Находим максимальную ширину текста всех кнопок
//    ImVec2 maxSize(0, 0);
//    const char* buttons[] = {
//        "Начать симуляцию",
//        "Остановить симуляцию",
//        "Шаг симуляции",
//        //"Предыдущее поколение", 
//        "Очистить поле",
//        "Случайные клетки",
//        "Спрятать сетку",
//        "Показать сетку",
//        "Сохранить",
//        "Загрузить",
//        "О программе",
//        "Выход"
//    };
//    for (int i = 0; i < IM_ARRAYSIZE(buttons); ++i) {
//        ImVec2 size = ImGui::CalcTextSize(buttons[i]);
//        if (size.x > maxSize.x) {
//            maxSize = size;
//        }
//    }
//    // Добавляем некоторый запас к ширине для лучшей визуализации
//    float buttonWidth = maxSize.x + 20; // 20 - это запас по ширине
//
//    if (gameController->isSimulationRunning()) {
//        if (ImGui::Button("Остановить симуляцию", ImVec2(buttonWidth, 0))) {
//            gameController->stopSimulation();
//        }
//    }
//    else {
//        if (ImGui::Button("Начать симуляцию", ImVec2(buttonWidth, 0))) {
//            gameController->startSimulation();
//        }
//    }
//    if (ImGui::Button("Шаг симуляции", ImVec2(buttonWidth, 0))) {
//        gameController->stepSimulation();
//    }
//    if (ImGui::Button("Очистить поле", ImVec2(buttonWidth, 0))) {
//        gameController->clearGrid();
//    }
//    if (ImGui::Button("Случайные клетки", ImVec2(buttonWidth, 0))) {
//        gameController->randomizeGrid(0.1f);
//    }
//    ImGui::Separator();
//    ImGui::Text("Фигура для добавления");
//    // Выпадающий список (ComboBox) с фиксированной шириной
//    static int selectedPattern = 0;
//    const char* patterns[] = { "Glider", "Blinker", "Toad", "Beacon", "Pentadecathlon", "Gosper Glider Gun" };
//    if (ImGui::Combo("##selectPattern", &selectedPattern, patterns, IM_ARRAYSIZE(patterns), static_cast<int>(buttonWidth))) {
//        gameController->setCurrentPattern(selectedPattern + 1); // +1 если нумерация начинается с 1
//    }
//
//    ImGui::Text("Вращение фигуры");
//    static int selectedPatternRotator = 0;
//    const char* patternsRotator[] = { "None", "Rotate 90", "Rotate 180", "Rotate 270", "Flip Horizontal", "Flip Vertical" };
//    if (ImGui::Combo("##selectRotator", &selectedPatternRotator, patternsRotator, IM_ARRAYSIZE(patternsRotator), static_cast<int>(buttonWidth))) {
//        gameController->setCurrentPatternRotator(selectedPatternRotator); // +1 если нумерация начинается с 1
//    }
//
//    if (gameController->getShowGrid()) {
//        if (ImGui::Button("Спрятать сетку", ImVec2(buttonWidth, 0))) {
//            gameController->setShowGrid(!gameController->getShowGrid());
//        }
//    }
//    else {
//        if (ImGui::Button("Показать сетку", ImVec2(buttonWidth, 0))) {
//            gameController->setShowGrid(!gameController->getShowGrid());
//        }
//    }
//    ImGui::Separator();
//    // В UI
//    if (gpuAutomaton) {
//        ImGui::Text("Правила игры:");
//        ImGui::Separator();
//        ImGui::Text("Рождение");
//        {
//            static int selectedBirth = 2;
//            static const char* birthOptions[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
//            if (ImGui::Combo("##birth", &selectedBirth , birthOptions, IM_ARRAYSIZE(birthOptions))) {
//                gpuAutomaton->birth = selectedBirth + 1;
//            }
//        }
//        ImGui::Separator();
//        ImGui::Text("Выживание (мин)");
//        {
//            static const char* survivalMinOptions[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
//            ImGui::Combo("##survivalMin", &gpuAutomaton->survivalMin, survivalMinOptions, IM_ARRAYSIZE(survivalMinOptions));
//        }
//        ImGui::Separator();
//        ImGui::Text("Выживание (макс)");
//        {
//            static const char* survivalMaxOptions[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
//            ImGui::Combo("##survivalMax", &gpuAutomaton->survivalMax, survivalMaxOptions, IM_ARRAYSIZE(survivalMaxOptions));
//        }
//        ImGui::Separator();
//        ImGui::Text("Перенаселение");
//        {
//            static int selectedOverPopulation = 3;
//            static const char* overpopulationOptions[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
//            if (ImGui::Combo( "##overpopulation", &selectedOverPopulation, overpopulationOptions, IM_ARRAYSIZE(overpopulationOptions)) ) {
//                gpuAutomaton->overpopulation = selectedOverPopulation + 1;
//            }
//
//        }
//    }
//    else {
//        ImGui::Text("GPUAutomaton не инициализирован!");
//    }
//    ImGui::Separator();
//    static char saveFilename[128] = "state.txt";
//    ImGui::Text("Имя файла");
//    // Поле ввода с фиксированной шириной
//    ImGui::PushItemWidth(buttonWidth);
//    ImGui::InputText("##fileName", saveFilename, IM_ARRAYSIZE(saveFilename));
//    ImGui::PopItemWidth();
//    if (ImGui::Button("Сохранить", ImVec2(buttonWidth, 0))) {
//        gameController->saveGameState(saveFilename);
//    }
//    if (ImGui::Button("Загрузить", ImVec2(buttonWidth, 0))) {
//        gameController->loadGameState(saveFilename);
//    }
//    ImGui::Separator();
//    for (int i = 0; i < 3; ++i) {
//        ImGui::Spacing();
//    }
//    if (ImGui::Button("О программе", ImVec2(buttonWidth, 0))) {
//        ImGui::OpenPopup("О программе");
//    }
//    for (int i = 0; i < 3; ++i) {
//        ImGui::Spacing();
//    }
//    if (ImGui::Button("Выход", ImVec2(buttonWidth, 0))) {
//        showExitDialog = true;
//    }
//    // Рендеринг диалога выхода
//    if (showExitDialog) {
//        ImGui::OpenPopup("Вы уверены?");
//        if (ImGui::BeginPopupModal("Вы уверены?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
//            ImGui::Text("Вы действительно хотите выйти?\n");
//
//            if (ImGui::Button("Да", ImVec2(120, 0))) {
//                PostQuitMessage(0);
//                showExitDialog = false;
//                ImGui::CloseCurrentPopup();
//            }
//            ImGui::SameLine();
//            if (ImGui::Button("Нет", ImVec2(120, 0))) {
//                showExitDialog = false;
//                ImGui::CloseCurrentPopup();
//            }
//            ImGui::EndPopup();
//        }
//    }
//    // Установка размера окна перед его отображением
//    ImGui::SetNextWindowSize(ImVec2(500.0f, 0.0f), ImGuiCond_Appearing); // Ширина 500 пикселей, высота автоматическая
//    // Модальное окно для справки
//    if (ImGui::BeginPopupModal("О программе", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
//        ImGui::TextWrapped("%s", aboutText.c_str());
//        
//
//        if (ImGui::Button("Закрыть", ImVec2(120, 0))) {
//            ImGui::CloseCurrentPopup();
//        }
//
//        ImGui::EndPopup();
//    }
//
//    ImGui::End();
//    // --------------------------------- конец главного меню игры -----------------------------
//    
//    ImGui::Render();
//    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//}


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

//void UIRenderer::DrawMenuBar() {
//
//    if (ImGui::BeginMainMenuBar())
//    {
//        if (ImGui::BeginMenu("File"))
//        {
//            if (ImGui::MenuItem("New")) {}
//            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
//            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
//            if (ImGui::MenuItem("Save As..")) {}
//
//            ImGui::Separator();
//
//            if (ImGui::MenuItem("Exit")) {}
//            ImGui::EndMenu();
//        }
//        if (ImGui::BeginMenu("Edit"))
//        {
//            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
//            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
//            ImGui::Separator();
//            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
//            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
//            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
//            ImGui::EndMenu();
//        }
//        if (ImGui::BeginMenu("Help"))
//        {
//            if (ImGui::MenuItem("About")) {}
//            ImGui::EndMenu();
//        }
//        ImGui::EndMainMenuBar();
//    }
//    
//}
void UIRenderer::DrawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Окна")) {
            ImGui::MenuItem("Управление симуляцией", NULL, &simulationWindowVisible);
            ImGui::MenuItem("Настройки игры", NULL, &gameSettingsWindowVisible);
            ImGui::MenuItem("Настройки сохранения", NULL, &saveSettingsWindowVisible);
            ImGui::MenuItem("Настройки поля", NULL, &fieldSettingsWindowVisible);
            ImGui::MenuItem("О программе", NULL, &aboutWindowVisible);
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Выход")) {
            exitDialogVisible = true;
        }
        ImGui::EndMainMenuBar();
    }
}

void UIRenderer::DrawSimulationWindow() {
    if (!simulationWindowVisible) return;

    ImGui::Begin("Управление симуляцией", &simulationWindowVisible, ImGuiWindowFlags_NoResize);
    //ImGui::SetWindowPos(ImVec2(2, 2), ImGuiCond_Once);
    ImGui::SetWindowSize(ImVec2(300, 400), ImGuiCond_Once);

    // Управление симуляцией
    if (gameController->isSimulationRunning()) {
        if (ImGui::Button("Остановить симуляцию")) {
            gameController->stopSimulation();
        }
    }
    else {
        if (ImGui::Button("Начать симуляцию")) {
            gameController->startSimulation();
        }
    }
    if (ImGui::Button("Шаг симуляции")) {
        gameController->stepSimulation();
    }
    if (ImGui::Button("Очистить поле")) {
        gameController->clearGrid();
    }
    if (ImGui::Button("Случайные клетки")) {
        gameController->randomizeGrid(0.1f);
    }

    // Выбор фигуры и поворот
    ImGui::Text("Фигура для добавления");
    static int selectedPattern = 0;
    const char* patterns[] = { "Glider", "Blinker", "Toad", "Beacon", "Pentadecathlon", "Gosper Glider Gun" };
    if (ImGui::Combo("##selectPattern", &selectedPattern, patterns, IM_ARRAYSIZE(patterns))) {
        gameController->setCurrentPattern(selectedPattern + 1);
    }

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

    ImGui::Begin("Настройки игры", &gameSettingsWindowVisible, ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize(ImVec2(300, 0), ImGuiCond_Once);

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

    ImGui::Begin("Настройки сохранения", &saveSettingsWindowVisible, ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize(ImVec2(300, 0), ImGuiCond_Once);

    ImGui::Text("Имя файла");
    ImGui::InputText("##fileName", saveFilename, IM_ARRAYSIZE(saveFilename));
    if (ImGui::Button("Сохранить")) {
        gameController->saveGameState(saveFilename);
    }
    if (ImGui::Button("Загрузить")) {
        gameController->loadGameState(saveFilename);
    }

    ImGui::End();
}

void UIRenderer::DrawFieldSettingsWindow() {
    if (!fieldSettingsWindowVisible) return;

    ImGui::Begin("Настройки поля", &fieldSettingsWindowVisible, ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize(ImVec2(200, 0), ImGuiCond_Once);

    ImGui::InputInt("Ширина", &fieldWidth);
    ImGui::InputInt("Высота", &fieldHeight);

    if (ImGui::Button("Применить")) {
        // Здесь нужно вызвать метод для изменения размеров поля в вашем gameController
        gameController->setFieldSize(fieldWidth, fieldHeight);
    }

    ImGui::End();
}

void UIRenderer::DrawAboutWindow() {
    if (!aboutWindowVisible) return;
    ImGui::SetNextWindowSize(ImVec2(500.0f, 0.0f), ImGuiCond_Appearing); // Ширина 500 пикселей, высота автоматическая
    ImGui::Begin("О программе", &aboutWindowVisible, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextWrapped("%s", aboutText.c_str());
    if (ImGui::Button("Закрыть")) {
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