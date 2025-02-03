#ifndef GAMEOFLIFE_H
#define GAMEOFLIFE_H

#include "Grid.h"
#include <vector>

class GameOfLife {
public:
    GameOfLife(Grid& g);
    void nextGeneration();
    void previousGeneration();
    void saveCurrentState();
    int countLiveNeighbors(int x, int y) const;
    int countLiveNeighborsWorld(int x, int y) const;
    void setWoldToroidal(bool wt) { isToroidal = wt; }
    bool getWoldToroidal() const { return isToroidal; }
    void updateGridReference(Grid& newGrid) { grid = newGrid; } // ����� ��� ���������� ������
private:
    Grid& grid;
    Grid nextGrid; // ��������� nextGrid ��� �������� ������
    std::vector<Grid> history;
    bool isToroidal = true;
};

#endif // GAMEOFLIFE_H

