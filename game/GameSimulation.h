#pragma once
#ifndef GAMESIMULATION_H
#define GAMESIMULATION_H

#include "GPUAutomaton.h"
#include "../rendering/IRendererProvider.h"

#include "Grid.h"
#include <vector>
#include <future>  

class GameSimulation {
public:
    GameSimulation(Grid& g);
    void nextGeneration();

	void setWoldToroidal(bool wt);
    bool getWoldToroidal() const { return isToroidal; };

    void setGpuSimulated(bool gs) { isGpuSimulated = gs; };
    bool getGpuSimulated() const { return isGpuSimulated; };

    void updateGridReference(Grid& newGrid); // метод для обновления ссылки
    void SetCellProvider(const IRendererProvider* provider);

    void SetCellColor(int x, int y, const Vector3d& color);

    GPUAutomaton& getGPUAutomaton() { return gpuAutomaton; }
    void updateStateInGPU();
private:
    Grid& grid;
    Grid nextGrid; // Добавляем nextGrid для двойного буфера
    GPUAutomaton gpuAutomaton; // член класса для вычислений на GPU
    bool gridReferenceIsUbdated;
    bool isToroidal;
    bool isGpuSimulated;
    bool isNideUpdate;
    int GW;
    int GH;
    std::vector<CellInstance>* cellInstances;
    const IRendererProvider* cellProvider;


    //std::atomic_flag gpuTaskReady = ATOMIC_FLAG_INIT;
    std::vector<int> currentState;
    std::vector<int> nextState;
    //std::future<void> gpuTaskFuture;

    void updateGridCpp(int* currentState, int* nextState, int GW, int GH);
};

#endif // GAMESIMULATION_H

