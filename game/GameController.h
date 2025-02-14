#pragma once
#ifndef GAMECONTROLLER_H_
#define GAMECONTROLLER_H_

#include "GameSimulation.h"
#include "Grid.h"
#include "GameStateManager.h"
#include "PatternManager.h"

#include <random> // ��� ��������� ��������� �����
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <filesystem>

// ��������� ��� ������ ��� ��������� ������
using Pattern = std::vector<std::vector<bool>>;

class GameController {
private:
    Grid grid;
    GameSimulation GameSimulation;
    float cellSize; // ������ ������ ������ � ��������
    bool isRunning; // ����, ����������, �������� �� ���������
    bool showGrid;
    bool showUI;
    float simulationSpeed = 0.01f; // �������� 1.0f ����� ��������������� ����� ������� ��������� �������
    float frameTimeAccumulator = 0.0f;

    std::vector<std::string> patternList;

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

    IRendererProvider* rendererProvider;

    Vector3d selectionStart; // ������ ���������� �������
    Vector3d selectionEnd;   // ����� ���������� �������
    bool isSelectionActive;  // ���� ���������� ���������

    std::vector<Vector3d> selectedCells; // ������ ��� �������� ���������� ������

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

    bool getShowUI() const { return showUI; };
    void setShowUI(bool isShow) { showUI = isShow;};

    bool getCellState(int x, int y) const;
    const Grid& getGrid() const { return grid; }

    // ����� ��� ��������� ������� �����
    void setFieldSize(int newWidth, int newHeight);

    void setCurrentPattern(int patternNumber);
    void setCurrentPatternRotator(int patternNumber);

    void PlacePattern(int startX, int startY);

    void setWoldToroidal(bool wt) { GameSimulation.setWoldToroidal( wt); };
    bool getWoldToroidal() const { return GameSimulation.getWoldToroidal(); };

    void setSimulationSpeed(float speed);

    // ����� ������ ��� ���������� � �������� ��������� ����
    bool saveGameState(const std::string& filename) const;
    bool loadGameState(const std::string& filename);
    bool saveGameStateCSV(const std::string& filename) const {
        return GameStateManager::saveGameStateCSV(grid, filename);
    }
    Pattern rotateOrFlip(const Pattern& pattern, Rotation rotation);

    void SetCellInstanceProvider(IRendererProvider* provider);

    GPUAutomaton& getGPUAutomaton() { return GameSimulation.getGPUAutomaton(); }

    // ����� ����� ��� �������� ������ ���������
    void loadPatternList(const std::string& patternFolder = "patterns");

    const std::vector<std::string>& getPatternList() const { return patternList; }

    void setCurrentPatternFromFile(const std::string& filename, int startX, int startY);

    void SetSelectionStart(int x, int y);
    void SetSelectionEnd(int x, int y);

    Vector3d GetSelectionStart() const { return selectionStart; }
    Vector3d GetSelectionEnd() const { return selectionEnd; }
    bool IsSelectionActive() const { return isSelectionActive; }

    void AddSelectedCell(int x, int y);
    void ClearSelectedCells();
    void KillSelectedCells();
    void ReviveSelectedCells();
    const std::vector<Vector3d>& GetSelectedCells() const { return selectedCells; }

};
#endif // GAMECONTROLLER_H_