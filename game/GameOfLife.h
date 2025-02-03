#ifndef GAMEOFLIFE_H
#define GAMEOFLIFE_H
#include "GPUAutomaton.h"
#include "Grid.h"
#include <vector>

class GameOfLife {
public:
    GameOfLife(Grid& g);
    void nextGeneration();
    void nextGenerationCPU();
    void previousGeneration();
    void saveCurrentState();
    int countLiveNeighbors(int x, int y) const;
    int countLiveNeighborsWorld(int x, int y) const;
    void setWoldToroidal(bool wt) { isToroidal = wt; }
    bool getWoldToroidal() const { return isToroidal; }
    void updateGridReference(Grid& newGrid) { grid = newGrid; } // метод для обновления ссылки
private:
    Grid& grid;
    Grid nextGrid; // Добавляем nextGrid для двойного буфера
    std::vector<Grid> history;
    bool isToroidal;
    GPUAutomaton gpuAutomaton; // член класса для вычислений на GPU
};

#endif // GAMEOFLIFE_H

