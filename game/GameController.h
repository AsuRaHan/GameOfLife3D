#pragma once
#include "GameOfLife.h"
#include "Grid.h"


class GameController {
private:
    Grid grid;
    GameOfLife gameOfLife;
    float cellSize; // Размер каждой клетки в пикселях
    bool isRunning; // Флаг, показывает, запущена ли симуляция

public:
    GameController(int width, int height, float cellSize = 0.5f);
    void initializeGrid();
    void randomizeGrid(float density);
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
};