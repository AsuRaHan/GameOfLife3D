#include "UIController.h"

UIController::UIController(GameController* gc) : gameController(gc), showExitDialog(false) {
    aboutText = LoadTextFromResource(GetModuleHandle(NULL), IDR_ABOUT_TEXT);
    gpuAutomaton = &gc->getGPUAutomaton();
}

void UIController::InitializeUI() {
    // Вы можете добавить здесь инициализацию или настройку UI, если это необходимо
}

std::string UIController::LoadTextFromResource(HINSTANCE hInstance, int resourceId) {
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

void UIController::DrawUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    //DrawMenuBar();
    // ------------------------------------ главное меню игры --------------------------------
    bool windowWasHovered = false;
    ImGui::SetNextWindowSize(ImVec2(0.0f, 650.0f), ImGuiCond_Always);
    ImGui::Begin("Управление игрой", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    // Проверяем, был ли окно под курсором мыши в прошлый раз
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow)) {
        windowWasHovered = true;
    }
    else if (windowWasHovered) {
        // Если мышь только что покинула область окна, снимаем фокус
        ImGui::SetWindowFocus(NULL); // NULL означает дефокусировку всех окон
        windowWasHovered = false;
    }
    ImGui::SetWindowPos(ImVec2(2, 2), ImGuiCond_Once);

    // Находим максимальную ширину текста всех кнопок
    ImVec2 maxSize(0, 0);
    const char* buttons[] = {
        "Начать симуляцию",
        "Остановить симуляцию",
        "Шаг симуляции",
        //"Предыдущее поколение", 
        "Очистить поле",
        "Случайные клетки",
        "Спрятать сетку",
        "Показать сетку",
        "Сохранить",
        "Загрузить",
        "О программе",
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

    if (gameController->isSimulationRunning()) {
        if (ImGui::Button("Остановить симуляцию", ImVec2(buttonWidth, 0))) {
            gameController->stopSimulation();
        }
    }
    else {
        if (ImGui::Button("Начать симуляцию", ImVec2(buttonWidth, 0))) {
            gameController->startSimulation();
        }
    }
    if (ImGui::Button("Шаг симуляции", ImVec2(buttonWidth, 0))) {
        gameController->stepSimulation();
    }
    if (ImGui::Button("Очистить поле", ImVec2(buttonWidth, 0))) {
        gameController->clearGrid();
    }
    if (ImGui::Button("Случайные клетки", ImVec2(buttonWidth, 0))) {
        gameController->randomizeGrid(0.1f);
    }
    ImGui::Separator();
    ImGui::Text("Фигура для добавления");
    // Выпадающий список (ComboBox) с фиксированной шириной
    static int selectedPattern = 0;
    const char* patterns[] = { "Glider", "Blinker", "Toad", "Beacon", "Pentadecathlon", "Gosper Glider Gun" };
    if (ImGui::Combo("##selectPattern", &selectedPattern, patterns, IM_ARRAYSIZE(patterns), static_cast<int>(buttonWidth))) {
        gameController->setCurrentPattern(selectedPattern + 1); // +1 если нумерация начинается с 1
    }

    ImGui::Text("Вращение фигуры");
    static int selectedPatternRotator = 0;
    const char* patternsRotator[] = { "None", "Rotate 90", "Rotate 180", "Rotate 270", "Flip Horizontal", "Flip Vertical" };
    if (ImGui::Combo("##selectRotator", &selectedPatternRotator, patternsRotator, IM_ARRAYSIZE(patternsRotator), static_cast<int>(buttonWidth))) {
        gameController->setCurrentPatternRotator(selectedPatternRotator); // +1 если нумерация начинается с 1
    }

    if (gameController->getShowGrid()) {
        if (ImGui::Button("Спрятать сетку", ImVec2(buttonWidth, 0))) {
            gameController->setShowGrid(!gameController->getShowGrid());
        }
    }
    else {
        if (ImGui::Button("Показать сетку", ImVec2(buttonWidth, 0))) {
            gameController->setShowGrid(!gameController->getShowGrid());
        }
    }
    ImGui::Separator();
    // В UI
    if (gpuAutomaton) {
        ImGui::Text("Правила игры:");
        ImGui::Separator();
        ImGui::Text("Рождение");
        {
            static const char* birthOptions[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
            ImGui::Combo("##birth", &gpuAutomaton->birth, birthOptions, IM_ARRAYSIZE(birthOptions));
        }
        ImGui::Separator();
        ImGui::Text("Выживание (мин)");
        {
            static const char* survivalMinOptions[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
            ImGui::Combo("##survivalMin", &gpuAutomaton->survivalMin, survivalMinOptions, IM_ARRAYSIZE(survivalMinOptions));
        }
        ImGui::Separator();
        ImGui::Text("Выживание (макс)");
        {
            static const char* survivalMaxOptions[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
            ImGui::Combo("##survivalMax", &gpuAutomaton->survivalMax, survivalMaxOptions, IM_ARRAYSIZE(survivalMaxOptions));
        }
        ImGui::Separator();
        ImGui::Text("Перенаселение");
        {
            static int selectedOverPopulation = 3;
            static const char* overpopulationOptions[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
            if (ImGui::Combo( "##overpopulation", &selectedOverPopulation, overpopulationOptions, IM_ARRAYSIZE(overpopulationOptions)) ) {
                gpuAutomaton->overpopulation = selectedOverPopulation + 1;
            }

        }
    }
    else {
        ImGui::Text("GPUAutomaton не инициализирован!");
    }
    ImGui::Separator();
    static char saveFilename[128] = "state.txt";
    ImGui::Text("Имя файла");
    // Поле ввода с фиксированной шириной
    ImGui::PushItemWidth(buttonWidth);
    ImGui::InputText("##fileName", saveFilename, IM_ARRAYSIZE(saveFilename));
    ImGui::PopItemWidth();
    if (ImGui::Button("Сохранить", ImVec2(buttonWidth, 0))) {
        gameController->saveGameState(saveFilename);
    }
    if (ImGui::Button("Загрузить", ImVec2(buttonWidth, 0))) {
        gameController->loadGameState(saveFilename);
    }
    ImGui::Separator();
    for (int i = 0; i < 3; ++i) {
        ImGui::Spacing();
    }
    if (ImGui::Button("О программе", ImVec2(buttonWidth, 0))) {
        ImGui::OpenPopup("О программе");
    }
    for (int i = 0; i < 3; ++i) {
        ImGui::Spacing();
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
    // Установка размера окна перед его отображением
    ImGui::SetNextWindowSize(ImVec2(500.0f, 0.0f), ImGuiCond_Appearing); // Ширина 500 пикселей, высота автоматическая
    // Модальное окно для справки
    if (ImGui::BeginPopupModal("О программе", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", aboutText.c_str());
        

        if (ImGui::Button("Закрыть", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
    // --------------------------------- конец главного меню игры -----------------------------
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIController::DrawMenuBar() {

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save As..")) {}

            ImGui::Separator();

            if (ImGui::MenuItem("Exit")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
}

void UIController::UpdateUIState() {
    // Это место для обновления состояния UI на основе изменений в игре или других событий
}