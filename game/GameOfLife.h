#pragma once
#ifndef GAMEOFLIFE_H
#define GAMEOFLIFE_H

#include "GPUAutomaton.h"
#include "../rendering/ICellInstanceProvider.h"

#include "Grid.h"
#include <vector>
#include <future>  

class GameOfLife {
public:
    GameOfLife(Grid& g);
    void nextGeneration();
    void previousGeneration();

	void setWoldToroidal(bool wt);
    bool getWoldToroidal() const { return isToroidal; };

    void updateGridReference(Grid& newGrid) { grid = newGrid; } // ����� ��� ���������� ������

    void SetCellProvider(const ICellInstanceProvider* provider);

    void SetCellColor(int x, int y, const Vector3d& color);

    GPUAutomaton& getGPUAutomaton() { return gpuAutomaton; }
private:
    Grid& grid;
    Grid nextGrid; // ��������� nextGrid ��� �������� ������
    GPUAutomaton gpuAutomaton; // ���� ������ ��� ���������� �� GPU

    std::vector<Grid> history;
    bool isToroidal;
    bool isGpuSimulated;
    
    std::vector<CellInstance>* cellInstances;
    const ICellInstanceProvider* cellProvider;

    void nextGenerationGPU();
    void nextGenerationCPU();
    
    void saveCurrentState();
    int countLiveNeighbors(int x, int y) const;
    int countLiveNeighborsWorldToroidal(int x, int y) const;
};

#endif // GAMEOFLIFE_H

