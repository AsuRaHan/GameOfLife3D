#pragma once
#include <vector>
#include "Grid.h"

class GameOfLife {
private:
    Grid& grid; // ������ �� �����, ����� �������� �����������
    std::vector<Grid> history; // �������� ������� ���������
    bool isToroidal = true;

    int countLiveNeighbors(int x, int y) const;
    int countLiveNeighborsWorld(int x, int y) const;
    void saveCurrentState();
public:
    GameOfLife(Grid& g);
    void nextGeneration();
    void previousGeneration();
    void updateGridReference(Grid& newGrid) { grid = newGrid; } // ����� ��� ���������� ������
    void setWoldToroidal(bool wt) { isToroidal = wt; };
    bool getWoldToroidal() { return isToroidal; };
};