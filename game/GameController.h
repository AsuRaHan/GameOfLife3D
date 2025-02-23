#pragma once
#ifndef GAMECONTROLLER_H_
#define GAMECONTROLLER_H_

//#include "GameSimulation.h"

#include "Grid.h"
#include "GPUAutomaton.h"
#include "GameStateManager.h"
#include "PatternManager.h"
#include "../mathematics/Vector3d.h"
#include "../rendering/IRendererProvider.h"

#include <random> // Для генерации случайных чисел
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <filesystem>

// Определим тип фигуры как двумерный массив
using Pattern = std::vector<std::vector<bool>>;

class GameController {
private:
    Grid grid;
    GPUAutomaton gpuAutomaton; // член класса для вычислений на GPU
    float cellSize; // Размер каждой клетки в пикселях
    bool isRunning; // Флаг, показывает, запущена ли симуляция
    bool showGrid;
    bool showUI;
    bool isWorldToroidal;
    int simulationSpeed = 0; // Значение 1000 может соответствовать одной секунде реального времени
    float frameTimeAccumulator = 0.0f;

    std::vector<std::string> patternList;

    // Определяем шаблоны фигур
    Pattern glider = {
        { 0, 1, 0 },
        { 0, 0, 1 },
        { 1, 1, 1 }
    };
    // Мигалка (Blinker) - Период 2
    Pattern blinker = {
    {1, 1, 1}
    };
    // Живок (Toad) - Период 2:
    Pattern toad = {
    {0, 1, 1, 1},
    {1, 1, 1, 0}
    };
    // Баржа (Beacon) - Период 2:
    Pattern beacon = {
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {0, 0, 1, 1},
    {0, 0, 1, 1}
    };
    // Маятник (Pentadecathlon) - Период 15:
    Pattern pentadecathlon = {
    {0,1,0},
    {0,1,0},
    {1,0,1},
    {0,1,0},
    {0,1,0},
    {0,1,0},
    {0,1,0},
    {1,0,1},
    {0,1,0},
    {0,1,0},
    };
    // Gosper Glider Gun - Период 30:
    Pattern gosperGliderGun = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    Pattern currentPattern; // Добавляем поле для хранения текущего паттерна
    int currentPatternRotator;
    enum class Rotation {
        Rotate90,      // Поворот на 90 градусов по часовой стрелке
        Rotate180,     // Поворот на 180 градусов
        Rotate270,     // Поворот на 270 градусов по часовой стрелке (или -90)
        FlipHorizontal, // Отражение по горизонтали
        FlipVertical    // Отражение по вертикали
    };

    IRendererProvider* rendererProvider;

    Vector3d selectionStart; // Начало выделяемой области
    Vector3d selectionEnd;   // Конец выделяемой области
    bool isSelectionActive;  // Флаг активности выделения

    std::vector<Vector3d> selectedCells; // Массив для хранения выделенных клеток
    int cellType = 1;
public:
    GameController(int width, int height, float cellSize = 0.5f);

    void randomizeGrid(float density);

    void placePattern(int startX, int startY, const Pattern& pattern);
    void PlacePattern(int startX, int startY);

    
    void clearGrid();
    void update(float deltaTime);
    void startSimulation();
    void stopSimulation();
    void stepSimulation();

    bool isSimulationRunning() const;

    void toggleCellState(int x, int y);
    void setLiveCell(int x, int y, bool state = true);
    float getCellSize() const { return cellSize; }
    int getGridWidth() const { return grid.getWidth(); }
    int getGridHeight() const { return grid.getHeight(); }
    bool getShowGrid() const {return showGrid;};
    void setShowGrid(bool isShow) {showGrid = isShow;};

    bool getShowUI() const { return showUI; };
    void setShowUI(bool isShow) { showUI = isShow;};

    bool getCellState(int x, int y) const;
    const Grid& getGrid() const { return grid; }

    // метод для изменения размера сетки
    void setFieldSize(int newWidth, int newHeight);

    void setCurrentPattern(int patternNumber);
    void setCurrentPatternRotator(int patternNumber);

    void setWoldToroidal(bool wt);
    bool getWoldToroidal() const { return isWorldToroidal; };

    void setSimulationSpeed(int speed);

    // Новые методы для сохранения и загрузки состояния игры
    bool saveGameState(const std::string& filename);
    bool loadGameState(const std::string& filename);

    Pattern rotateOrFlip(const Pattern& pattern, Rotation rotation);

    void SetRendererProvider(IRendererProvider* provider);

    GPUAutomaton& getGPUAutomaton() { return gpuAutomaton; }

    // Новый метод для загрузки списка паттернов
    void loadPatternList(const std::string& patternFolder = "patterns");

    const std::vector<std::string>& getPatternList() const { return patternList; }

    void setCurrentPatternFromFile(const std::string& filename, int startX, int startY);

    void SetSelectionStart(int x, int y);
    void SetSelectionEnd(int x, int y);

    Vector3d GetSelectionStart() const { return selectionStart; }
    Vector3d GetSelectionEnd() const { return selectionEnd; }
    bool IsSelectionActive() const { return isSelectionActive; }
    void setlectionActive(bool isActive) { isSelectionActive = isActive; }

    void AddSelectedCell(int x, int y);
    void ClearSelectedCells();
    void KillSelectedCells();
    void ReviveSelectedCells();
    const std::vector<Vector3d>& GetSelectedCells() const { return selectedCells; }

    void SaveSelectedCellsAsPattern();
    void SetCellType(int type); // метод для установки типа клетки
};
#endif // GAMECONTROLLER_H_