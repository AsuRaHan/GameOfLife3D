#include "GameOfLife.h"

GameOfLife::GameOfLife(Grid& g) : grid(g), nextGrid(g.getWidth(), g.getHeight()),
gpuAutomaton(g.getWidth(), g.getHeight()), isToroidal(true), isGpuSimulated(true),
cellInstances(nullptr),cellProvider(nullptr), gridReferenceIsUbdated(true)
{
    GW = grid.getWidth();
    GH = grid.getHeight();
    gpuAutomaton.SetToroidal(isToroidal); // Установка isToroidal
    gpuAutomaton.SetNewGridSize(GW, GH);
    currentState.resize(GW * GH);
    nextState.resize(GW * GH);
    
}

int GameOfLife::countLiveNeighbors(int x, int y) const {
    constexpr int offsets[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };

    int count = 0;
    for (const auto& offset : offsets) {
        int nx = x + offset[0];
        int ny = y + offset[1];

        // Проверяем, находятся ли координаты в пределах границ сетки
        if (nx >= 0 && nx < grid.getWidth() && ny >= 0 && ny < grid.getHeight()) {
            if (grid.getCellState(nx, ny)) {
                count++;
            }
        }
    }
    return count;
}

int GameOfLife::countLiveNeighborsWorldToroidal(int x, int y) const {
    constexpr int offsets[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };

    int count = 0;
    const int w = grid.getWidth();
    const int h = grid.getHeight();

    for (const auto& offset : offsets) {
        int nx = x + offset[0];
        int ny = y + offset[1];

        // Корректировка координат для тороидальной сетки без использования wrap
        nx = (nx < 0) ? w - 1 : (nx >= w ? 0 : nx);
        ny = (ny < 0) ? h - 1 : (ny >= h ? 0 : ny);

        if (grid.getCellState(nx, ny)) {
            count++;
        }
    }
    return count;
}

void GameOfLife::setWoldToroidal(bool wt) {
    isToroidal = wt;
    gpuAutomaton.SetToroidal(isToroidal); // Установка isToroidal в GPUAutomaton
}

void GameOfLife::nextGeneration() {
    if (isGpuSimulated) {
        nextGenerationGPU();
    }
    else {
        nextGenerationCPU();
    }
}

void GameOfLife::updateGridReference(Grid& newGrid) {
    grid = newGrid;
    GW = grid.getWidth();
    GH = grid.getHeight();

    currentState.resize(GW * GH);
    nextState.resize(GW * GH);
    gpuAutomaton.SetNewGridSize(GW, GH);

}

void GameOfLife::nextGenerationGPU() {
    //for (int y = 0; y < GH; ++y) {
    //    for (int x = 0; x < GW; ++x) {
    //        currentState[y * GW + x] = grid.getCellState(x, y) ? 1 : 0;
    //    }
    //}

    //Асинхронная загрузка данных на GPU
    auto future = std::async(std::launch::async, [&]() {
        for (int y = 0; y < GH; ++y) {
            for (int x = 0; x < GW; ++x) {
                currentState[y * GW + x] = grid.getCellState(x, y) ? 1 : 0;
            }
        }
        });

    // Другие операции пока данные загружаются
    future.wait(); // Ждем завершения загрузки

    gpuAutomaton.SetGridState(currentState);
    // Выполняем обновление на GPU
    gpuAutomaton.Update();
    // Получаем новое состояние с GPU
    gpuAutomaton.GetGridState(nextState);


    for (auto y = 0; y < GH; ++y) {
        for (auto x = 0; x < GW; ++x) {
            bool newState = nextState[y * GW + x] != 0;
            Cell& cell = grid.getCell(x, y);
            bool oldState = cell.getAlive();

            if (newState != oldState) { // Только если состояние изменилось
                cell.setAlive(newState);
                if (newState) {
                    // Вновь рожденные клетки получают темный цвет
                    cell.setColor(Vector3d(0.0f, 0.4f, 0.0f)); // рождение
                }
                else {
                    // Умершие клетки получают темно-серый цвет
                    cell.setColor(Vector3d(0.05f, 0.05f, 0.08f)); // смерть
                }
                SetCellColor(x, y, cell.getColor());
            }
            else if (newState) {
                // Если клетка пережила шаг, делаем её светлее
                Vector3d currentColor = cell.getColor();
                // Увеличиваем все компоненты цвета на небольшое значение, но не более 1.0
                float newR = currentColor.X() + 0.05f;
                float newG = currentColor.Y() + 0.05f;
                float newB = currentColor.Z() + 0.05f;
                newR = newR > 0.3f ? 0.3f : newR;
                newG = newG > 0.9f ? 0.9f : newG;
                newB = newB > 0.3f ? 0.3f : newB;
                cell.setColor(Vector3d(newR, newG, newB));
                SetCellColor(x, y, cell.getColor());
            }
        }
    }

}

void GameOfLife::SetCellColor(int x, int y, const Vector3d& color) {
    if (cellProvider && cellInstances) {
        //int GW = grid.getWidth();
        int index = y * GW + x;
        if (index < cellInstances->size()) {
            (*cellInstances)[index].color = color;
        }
    }
}

void GameOfLife::SetCellProvider(const IRendererProvider* provider) {
    cellProvider = provider;
    if (cellProvider) {
        cellInstances = &const_cast<std::vector<CellInstance>&>(cellProvider->GetCellInstances());
    }
    else {
        cellInstances = nullptr;
    }
}

void GameOfLife::nextGenerationCPU() {
    //saveCurrentState(); // Сохраняем текущее состояние
    int neighbors;
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            if (isToroidal) {                
                neighbors = countLiveNeighborsWorldToroidal(x, y);
            }
            else {
                neighbors = countLiveNeighbors(x, y);
            }

            bool currentState = grid.getCellState(x, y);
            Cell& oldCell = grid.getCell(x, y); // Ссылка на старую клетку
            Cell newCell = oldCell; // Копируем старую клетку

            if (currentState) {
                // Живая клетка
                newCell.setAlive(neighbors == 2 || neighbors == 3);
                if (!newCell.getAlive()) {
                    // Если клетка умирает, используем темно-серый цвет
                    newCell.setColor(Vector3d(0.25f, 0.25f, 0.25f));
                }
                else {
                    // Если клетка остается живой, делаем её зеленог цвета
                    newCell.setColor(Vector3d(0.3f, 0.8f, 0.3f));
                }
            }
            else {
                // Мертвая клетка
                newCell.setAlive(neighbors == 3);
                if (newCell.getAlive()) {
                    // Если клетка рождается, делаем её темно зеленого цвета
                    newCell.setColor(Vector3d(0.0f, 0.5f, 0.0f)); // зеленый цвет
                }
            }
            nextGrid.setCell(x, y, newCell); // Устанавливаем новую клетку
        }
    }
    std::swap(grid, nextGrid); // Переключаем буферы
}

void GameOfLife::previousGeneration() {
    if (!history.empty()) {
        grid = history.back(); // Восстанавливаем предыдущее состояние
        history.pop_back(); // Удаляем последнее сохраненное состояние из истории
    }
    else {
        // Обработка случая, когда нет предыдущих состояний
        //std::cout << "Нет предыдущего поколения для восстановления." << std::endl;
    }
}

void GameOfLife::saveCurrentState() {
    if (history.size() >= 100) {
        history.erase(history.begin()); // Удаляем самое старое состояние
    }
    history.push_back(grid); // Сохраняем текущее состояние
}