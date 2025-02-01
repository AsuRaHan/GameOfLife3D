#pragma once
#include <vector>
#include "Grid.h"

class GameOfLife {
private:
    Grid& grid; // Ссылка на сетку, чтобы избежать копирования
    std::vector<Grid> history; // Хранение истории состояний

    int countLiveNeighbors(int x, int y) const;
    void saveCurrentState();
public:
    GameOfLife(Grid& g);
    void nextGeneration();
    void previousGeneration();
    void updateGridReference(Grid& newGrid) { grid = newGrid; } // метод для обновления ссылки
};