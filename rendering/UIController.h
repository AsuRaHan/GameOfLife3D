#pragma once
#ifndef UICONTROLLER_H_
#define UICONTROLLER_H_

#include "../game/GameController.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"

class UIController {
private:
    GameController* gameController;

    bool showExitDialog;
    const char* aboutText = R"(
Добро пожаловать в справку по использованию программы!
    Здесь вы можете найти инструкции по работе с различными функциями:
    - Начать симуляцию: Нажмите кнопку 'Начать симуляцию' для запуска жизни.
    - Остановить симуляцию: Нажмите 'Остановить симуляцию', чтобы приостановить процесс.
    - Шаг симуляции: Используйте 'Шаг симуляции' для ручного продвижения на одно поколение.
    - Очистить поле: Кнопка 'Очистить поле' очищает всю сетку.
    - Случайные клетки: 'Случайные клетки' заполняет сетку случайным образом.
    - Управление сеткой: Используйте 'Спрятать сетку' или 'Показать сетку' для изменения видимости сетки.
    - Сохранение/Загрузка: 'Сохранить' и 'Загрузить' позволяют работать с состояниями игры.
    "Для выхода из программы нажмите 'Выход'.
)";

public:
    UIController(GameController* gc);
    void InitializeUI();
    void DrawUI();
    void DrawMenuBar();
    void UpdateUIState();
};

#endif // UICONTROLLER_H_