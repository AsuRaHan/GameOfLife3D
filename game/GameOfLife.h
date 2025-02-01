#pragma once
#include <vector>
#include "Grid.h"

class GameOfLife {
private:
    Grid& grid; // ������ �� �����, ����� �������� �����������
    std::vector<Grid> history; // �������� ������� ���������

    int countLiveNeighbors(int x, int y) const;
    void saveCurrentState();
public:
    GameOfLife(Grid& g);
    void nextGeneration();
    void previousGeneration();
    void updateGridReference(Grid& newGrid) { grid = newGrid; } // ����� ��� ���������� ������
};