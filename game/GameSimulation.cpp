#include "GameSimulation.h"

GameSimulation::GameSimulation(Grid& g) : grid(g), nextGrid(g.getWidth(), g.getHeight()),
gpuAutomaton(g.getWidth(), g.getHeight()), isToroidal(true),
cellInstances(nullptr),cellProvider(nullptr), gridReferenceIsUbdated(true)
{
    GW = grid.getWidth();
    GH = grid.getHeight();
    gpuAutomaton.SetToroidal(isToroidal); // Установка isToroidal
    gpuAutomaton.SetNewGridSize(GW, GH);
    currentState.resize(GW * GH);
    nextState.resize(GW * GH);
    grid.SetGPUAutomaton(&gpuAutomaton);
}

void GameSimulation::setWoldToroidal(bool wt) {
    isToroidal = wt;
    gpuAutomaton.SetToroidal(isToroidal); // Установка isToroidal в GPUAutomaton
}


void GameSimulation::nextGeneration() {

    //if (true) {
        //if (isNideUpdate) {
        //    updateStateInGPU();
        //    isNideUpdate = false;
        //}
        //Выполняем обновление на GPU
        gpuAutomaton.Update();
        //Получаем новое состояние с GPU
        //gpuAutomaton.GetGridState(nextState);
}

// Реализация нового метода на C++
void GameSimulation::updateGridCpp(int* currentState, int* nextState, int GW, int GH) {
    for (int y = 0; y < GH; ++y) {
        for (int x = 0; x < GW; ++x) {
            // Подсчитываем количество живых соседей
            int count = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue; // Пропускаем текущую клетку

                    int nx = x + dx;
                    int ny = y + dy;

                    if (isToroidal) {
                        // Тороидальная коррекция
                        nx = (nx + GW) % GW;
                        ny = (ny + GH) % GH;
                    }
                    else {
                        // Проверка границ для нетороидального мира
                        if (nx < 0 || nx >= GW || ny < 0 || ny >= GH) continue;
                    }

                    if (currentState[ny * GW + nx] != 0) {
                        count++;
                    }
                }
            }

            // Получаем текущее состояние
            int current = currentState[y * GW + x];

            // Применяем правила "Игры Жизни"
            int next = 0;
            if (current != 0) {
                // Живая клетка
                if (count >= gpuAutomaton.survivalMin && count <= gpuAutomaton.survivalMax) {
                    next = 1; // Клетка выживает
                }
            }
            else {
                // Мертвая клетка
                if (count == gpuAutomaton.birth) {
                    next = 1; // Клетка оживает
                }
            }

            // Сохраняем новое состояние
            nextState[y * GW + x] = next;
        }
    }
}

void GameSimulation::updateGridReference(Grid& newGrid) {
    grid = newGrid;
    GW = grid.getWidth();
    GH = grid.getHeight();

    currentState.resize(GW * GH);
    nextState.resize(GW * GH);
    gpuAutomaton.SetNewGridSize(GW, GH);
}

//void GameSimulation::SetCellColor(int x, int y, const Vector3d& color) {
//    if (cellProvider && cellInstances) {
//        int index = y * GW + x;
//        if (index < cellInstances->size()) {
//            //(*cellInstances)[index].color = color;
//        }
//    }
//}

void GameSimulation::SetCellProvider(const IRendererProvider* provider) {
    cellProvider = provider;
    if (cellProvider) {
        cellInstances = &const_cast<std::vector<CellInstance>&>(cellProvider->GetCellInstances());
    }
    else {
        cellInstances = nullptr;
    }
}