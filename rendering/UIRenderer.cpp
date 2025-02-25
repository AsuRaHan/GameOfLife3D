#include "UIRenderer.h"

UIRenderer::UIRenderer(GameController* gc) : gameController(gc), showExitDialog(false) {
    aboutText = LoadTextFromResource(GetModuleHandle(NULL), IDR_ABOUT_TEXT);
    gpuAutomaton = &gc->getGPUAutomaton();
    buttonSize = ImVec2(200, 30);
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
    DrawPatternWindow(); 

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIRenderer::DrawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        // Меню "Окна" слева
        if (ImGui::BeginMenu("Окна")) {
            ImGui::MenuItem("Управление симуляцией", NULL, &simulationWindowVisible);
            ImGui::MenuItem("Правила жизни/смерти", NULL, &gameSettingsWindowVisible);
            ImGui::MenuItem("Сохранения и загрузка", NULL, &saveSettingsWindowVisible);
            ImGui::MenuItem("Настройки поля", NULL, &fieldSettingsWindowVisible);
            ImGui::MenuItem("Паттерны", NULL, &patternWindowVisible);
            ImGui::MenuItem("О программе", NULL, &aboutWindowVisible);
            ImGui::EndMenu();
        }

        // Получаем статистику
        std::string statsText = PerformanceStats::getInstance().getStatsString();

        // Вычисляем ширину меню-бара и текста
        float menuBarWidth = ImGui::GetWindowWidth();
        float exitTextWidth = ImGui::CalcTextSize("Выход").x;
        float statsTextWidth = ImGui::CalcTextSize(statsText.c_str()).x;
        float spacing = ImGui::GetStyle().ItemSpacing.x;

        // Устанавливаем позицию для статистики (перед "Выход")
        ImGui::SetCursorPosX(menuBarWidth - exitTextWidth - statsTextWidth - 2 * spacing - 10.0f);
        ImGui::Text("%s", statsText.c_str()); // Выводим статистику

        // Кнопка "Выход" справа
        ImGui::SetCursorPosX(menuBarWidth - exitTextWidth - spacing - 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f)); // Красный цвет для "Выход"
        if (ImGui::MenuItem("Выход")) {
            exitDialogVisible = true;
        }
        ImGui::PopStyleColor();

        ImGui::EndMainMenuBar();
    }
}

void UIRenderer::DrawSimulationWindow() {
    if (!simulationWindowVisible) return;

    ImGui::Begin("Симуляция", &simulationWindowVisible, ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize(ImVec2(0, 250), ImGuiCond_Once);
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

    if (gameController->getWoldToroidal()) {
        if (ImGui::Button("Ограничить мир", buttonSize)) {
            if (!gameController->isSimulationRunning()) {
                gameController->setWoldToroidal(false); // сделать мир безграничным или на оборот ограничеть его
            }
        }
    }
    else {
        if (ImGui::Button("Разграничить мир", buttonSize)) {
            if (!gameController->isSimulationRunning()) {
                gameController->setWoldToroidal(true);
            }
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
    ImGui::Text("Задержка симуляции");
    static int simulateTime = 0.0f;
    ImGui::SetNextItemWidth(buttonSize.x);
    if (ImGui::SliderInt("##simulateTime", &simulateTime, 0, 1000)) {
        // Здесь код выполнится, если значение слайдера изменилось
        gameController->setSimulationSpeed(simulateTime);
    }


    ImGui::End();
}


void UIRenderer::DrawGameSettingsWindow() {
    if (!gameSettingsWindowVisible) return;

    ImGui::Begin("Правила жизни/смерти", &gameSettingsWindowVisible, ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize(ImVec2(250, 0), ImGuiCond_Once);
    if (gpuAutomaton) {
        ImGui::Text("Правила игры:");
        ImGui::Separator();

        // Чекбокс для расширенного режима
        static bool advancedRules = false;
        if (ImGui::Checkbox("Расширенные правила (B/S)", &advancedRules)) {
            gpuAutomaton->setUseAdvancedRules(advancedRules);
        }

        // Если расширенный режим включен, отключаем старые комбобоксы
        if (advancedRules) {
            ImGui::BeginDisabled();
        }

        // Старые настройки
        ImGui::Text("Рождение");
        int selectedBirth = gpuAutomaton->birth - 1;
        const char* birthOptions[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
        ImGui::SetNextItemWidth(buttonSize.x);
        if (ImGui::Combo("##birth", &selectedBirth, birthOptions, IM_ARRAYSIZE(birthOptions))) {
            gpuAutomaton->birth = selectedBirth + 1;
        }
        ImGui::Separator();
        ImGui::Text("Выживание (мин)");
        const char* survivalMinOptions[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
        ImGui::SetNextItemWidth(buttonSize.x);
        ImGui::Combo("##survivalMin", &gpuAutomaton->survivalMin, survivalMinOptions, IM_ARRAYSIZE(survivalMinOptions));
        ImGui::Separator();
        ImGui::Text("Выживание (макс)");
        const char* survivalMaxOptions[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
        ImGui::SetNextItemWidth(buttonSize.x);
        ImGui::Combo("##survivalMax", &gpuAutomaton->survivalMax, survivalMaxOptions, IM_ARRAYSIZE(survivalMaxOptions));
        ImGui::Separator();
        ImGui::Text("Перенаселение");
        int selectedOverPopulation = gpuAutomaton->overpopulation - 1;
        const char* overpopulationOptions[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
        ImGui::SetNextItemWidth(buttonSize.x);
        if (ImGui::Combo("##overpopulation", &selectedOverPopulation, overpopulationOptions, IM_ARRAYSIZE(overpopulationOptions))) {
            gpuAutomaton->overpopulation = selectedOverPopulation + 1;
        }

        if (advancedRules) {
            ImGui::EndDisabled();
        }

        // Окно для расширенных правил
        if (advancedRules) {
            ImGui::Begin("Расширенные правила", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::SetWindowSize(ImVec2(250, 0), ImGuiCond_Once);
            static bool birthRules[9] = { false };
            static bool surviveRules[9] = { false };

            // Рождение
            ImGui::Text("Рождение (B):");
            ImGui::Separator();
            for (int i = 0; i <= 8; i++) {
                ImGui::BeginGroup(); // Группа для каждого чекбокса
                ImGui::Text("%d соседей", i);
                ImGui::Checkbox(("##birth" + std::to_string(i)).c_str(), &birthRules[i]);
                ImGui::EndGroup();
                if (i % 3 != 2) ImGui::SameLine(); // 3 чекбокса в ряд
            }

            ImGui::Spacing(); // Отступ между секциями
            ImGui::Separator();

            // Выживание
            ImGui::Text("Выживание (S):");
            ImGui::Separator();
            for (int i = 0; i <= 8; i++) {
                ImGui::BeginGroup();
                ImGui::Text("%d соседей", i);
                ImGui::Checkbox(("##survive" + std::to_string(i)).c_str(), &surviveRules[i]);
                ImGui::EndGroup();
                if (i % 3 != 2) ImGui::SameLine(); // 3 чекбокса в ряд
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Перенаселение (O):");
            ImGui::Separator();
            static bool overpopulationRules[9] = { false };
            for (int i = 1; i <= 8; i++) { // Начинаем с 1, пропускаем 0
                ImGui::BeginGroup();
                ImGui::Text("%d соседей", i);
                ImGui::Checkbox(("##overpop" + std::to_string(i)).c_str(), &overpopulationRules[i]);
                ImGui::EndGroup();
                if (i % 3 != 0) ImGui::SameLine(); // 3 в ряд, но учитываем, что начинаем с 1
            }
            ImGui::NewLine();
            // Кнопка применения
            if (ImGui::Button("Применить")) {
                gpuAutomaton->setBirthRules(birthRules);
                gpuAutomaton->setSurviveRules(surviveRules);
                gpuAutomaton->setOverpopulationRules(overpopulationRules);
            }
            ImGui::End();
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

    if (ImGui::Button("Перестроить поле", buttonSize)) {
        // Здесь нужно вызвать метод для изменения размеров поля в вашем gameController
        gameController->setFieldSize(fieldWidth, fieldHeight);
    }

    ImGui::End();
}

void UIRenderer::DrawAboutWindow() {
    if (!aboutWindowVisible) return;
    ImGui::SetNextWindowSize(ImVec2(700.0f, 0.0f), ImGuiCond_Appearing); // Ширина 600 пикселей, высота автоматическая
    ImGui::Begin("О программе", &aboutWindowVisible);
    ImGui::TextWrapped("%s", aboutText.c_str());
    //if (ImGui::Button("Закрыть", buttonSize)) {
    //    aboutWindowVisible = false;
    //}
    ImGui::End();
}

void UIRenderer::DrawExitDialog() {
    if (!exitDialogVisible) return;

    ImGui::OpenPopup("Подтверждение выхода");
    if (ImGui::BeginPopupModal("Подтверждение выхода", NULL,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("Вы уверены, что хотите выйти?");
        ImGui::Separator();
        if (ImGui::Button("Да", ImVec2(120, 0))) {
            // Закрытие приложения
            PostQuitMessage(0);
        }
        ImGui::SameLine();
        if (ImGui::Button("Нет", ImVec2(120, 0))) {
            exitDialogVisible = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

}

void UIRenderer::DrawPatternWindow() {
    if (!patternWindowVisible) return;

    ImGui::Begin("паттерны", &patternWindowVisible, ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize(ImVec2(0, 500), ImGuiCond_Once);
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
    if (ImGui::Button("Загрузить список", buttonSize)) {
        gameController->loadPatternList();
        patternListLoaded = true;
        selectedPatternIndex = -1; // Сбрасываем выбор при новой загрузке
    }
    ImGui::Separator();
    ImGui::Text("Текущий паттерн: %s", currentPatternName.c_str());
    ImGui::Separator();
    ImGui::InputText("Поиск", searchFilter, IM_ARRAYSIZE(searchFilter));
    if (patternListLoaded) {
        const auto& patterns = gameController->getPatternList();
        if (patterns.empty()) {
            ImGui::Text("Список паттернов пуст.");
            if (ImGui::Button("Скачать паттерны", buttonSize)) {
                const char* url = "https://conwaylife.com/patterns/all.zip";
                ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
            }
        }
        else {

            // Начинаем область с прокруткой
            ImGui::BeginChild("PatternList", ImVec2(0, 270), false);
            for (size_t i = 0; i < patterns.size(); ++i) {
                std::string itemName = std::filesystem::path(patterns[i]).filename().string();
                // Фильтруем по имени файла
                std::string filterLower = searchFilter;
                std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
                std::string itemNameLower = itemName;
                std::transform(itemNameLower.begin(), itemNameLower.end(), itemNameLower.begin(), ::tolower);

                if (itemNameLower.find(filterLower) != std::string::npos) {
                    if (ImGui::Selectable(itemName.c_str(), selectedPatternIndex == static_cast<int>(i))) {
                        selectedPatternIndex = static_cast<int>(i);
                        currentPatternName = itemName;
                        ImGui::SetWindowFocus(nullptr);
                        gameController->setCurrentPatternFromFile(gameController->getPatternList()[selectedPatternIndex], gameController->getGridWidth() / 2, gameController->getGridHeight() / 2);
                    }
                }
            }
            ImGui::EndChild(); // Заканчиваем область с прокруткой
        }
    }

    ImGui::End();

}