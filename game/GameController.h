#pragma once
#include "GameOfLife.h"
#include "Grid.h"
#include <random> // Для генерации случайных чисел
#include <string>
#include <fstream>
#include <sstream>

class GameController {
private:
    Grid grid;
    GameOfLife gameOfLife;
    float cellSize; // Размер каждой клетки в пикселях
    bool isRunning; // Флаг, показывает, запущена ли симуляция

public:
    GameController(int width, int height, float cellSize = 0.5f);
    void initializeGrid();
    void placeGlider(int startX, int startY);
    void randomizeGrid(float density);
    void clearGrid();
    void update();
    void startSimulation();
    void stopSimulation();
    void stepSimulation();
    void previousGeneration();
    bool isSimulationRunning() const;

    void toggleCellState(int x, int y);
    float getCellSize() const { return cellSize; }
    int getGridWidth() const { return grid.getWidth(); }
    int getGridHeight() const { return grid.getHeight(); }

    bool getCellState(int x, int y) const;
    const Grid& getGrid() const { return grid; }

    void saveGameState(const std::string& filename);
    void loadGameState(const std::string& filename);
    void saveGameStateCSV(const std::string& filename);
    void loadGameStateCSV(const std::string& filename);

    // метод для изменения размера сетки
    void resizeGrid(int newWidth, int newHeight);
};