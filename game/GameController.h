#pragma once
#ifndef GAMECONTROLLER_H_
#define GAMECONTROLLER_H_

#include "GameOfLife.h"
#include "Grid.h"
#include "GameStateManager.h"
//#include "../rendering/ICellInstanceProvider.h"

#include <random> // ��� ��������� ��������� �����
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>

// ��������� ��� ������ ��� ��������� ������
using Pattern = std::vector<std::vector<bool>>;

class GameController {
private:
    Grid grid;
    GameOfLife gameOfLife;
    float cellSize; // ������ ������ ������ � ��������
    bool isRunning; // ����, ����������, �������� �� ���������
    bool showGrid;
    float simulationSpeed = 0.01f; // �������� 1.0f ����� ��������������� ����� ������� ��������� �������
    float frameTimeAccumulator = 0.0f;

    // ���������� ������� �����
    Pattern glider = {
        { 0, 1, 0 },
        { 0, 0, 1 },
        { 1, 1, 1 }
    };
    // ������� (Blinker) - ������ 2
    Pattern blinker = {
    {1, 1, 1}
    };
    // ����� (Toad) - ������ 2:
    Pattern toad = {
    {0, 1, 1, 1},
    {1, 1, 1, 0}
    };
    // ����� (Beacon) - ������ 2:
    Pattern beacon = {
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {0, 0, 1, 1},
    {0, 0, 1, 1}
    };
    // ������� (Pentadecathlon) - ������ 15:
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
    // Gosper Glider Gun - ������ 30:
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

    Pattern currentPattern; // ��������� ���� ��� �������� �������� ��������
    int currentPatternRotator;
    enum class Rotation {
        Rotate90,      // ������� �� 90 �������� �� ������� �������
        Rotate180,     // ������� �� 180 ��������
        Rotate270,     // ������� �� 270 �������� �� ������� ������� (��� -90)
        FlipHorizontal, // ��������� �� �����������
        FlipVertical    // ��������� �� ���������
    };
public:
    GameController(int width, int height, float cellSize = 0.5f);
    void randomizeGrid();
    void placePattern(int startX, int startY, const Pattern& pattern);
    void randomizeGrid(float density);
    void clearGrid();
    void update(float deltaTime);
    void startSimulation();
    void stopSimulation();
    void stepSimulation();
    void previousGeneration();
    bool isSimulationRunning() const;

    void toggleCellState(int x, int y);
    float getCellSize() const { return cellSize; }
    int getGridWidth() const { return grid.getWidth(); }
    int getGridHeight() const { return grid.getHeight(); }
    bool getShowGrid() const {return showGrid;};
    void setShowGrid(bool isShow) {showGrid = isShow;};

    bool getCellState(int x, int y) const;
    const Grid& getGrid() const { return grid; }

    // ����� ��� ��������� ������� �����
    void setFieldSize(int newWidth, int newHeight);

    void setCurrentPattern(int patternNumber);
    void setCurrentPatternRotator(int patternNumber);

    void PlacePattern(int startX, int startY);

    void setWoldToroidal(bool wt) { gameOfLife.setWoldToroidal( wt); };
    bool getWoldToroidal() const { return gameOfLife.getWoldToroidal(); };

    void setSimulationSpeed(float speed);

    // ����� ������ ��� ���������� � �������� ��������� ����
    bool saveGameState(const std::string& filename) const;
    bool loadGameState(const std::string& filename);
    bool saveGameStateCSV(const std::string& filename) const {
        return GameStateManager::saveGameStateCSV(grid, filename);
    }
    Pattern rotateOrFlip(const Pattern& pattern, Rotation rotation);

    void SetCellInstanceProvider(ICellInstanceProvider* provider) {
        gameOfLife.SetCellProvider(provider); // �������� ���������� � GameOfLife
    }

    GPUAutomaton& getGPUAutomaton() { return gameOfLife.getGPUAutomaton(); }
};
#endif // GAMECONTROLLER_H_