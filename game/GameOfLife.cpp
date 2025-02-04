#include "GameOfLife.h"

GameOfLife::GameOfLife(Grid& g) : grid(g), nextGrid(g.getWidth(), g.getHeight()),gpuAutomaton(g.getWidth(), g.getHeight()), isToroidal(true) {
    //std::srand(static_cast<unsigned int>(std::time(nullptr))); // Инициализация генератора случайных чисел
}

int GameOfLife::countLiveNeighbors(int x, int y) const {
    static const int offsets[8][2] = {
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

int GameOfLife::countLiveNeighborsWorld(int x, int y) const {
    static const int offsets[8][2] = {
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
    gpuAutomaton.SetToroidal(wt); // Установка isToroidal в GPUAutomaton
}

void GameOfLife::nextGeneration() {
    // Используем GPU для вычислений нового поколения
    std::vector<int> currentState, nextState;
    currentState.resize(grid.getWidth() * grid.getHeight());
    nextState.resize(grid.getWidth() * grid.getHeight());

    // Преобразование состояния сетки в вектор для передачи на GPU
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            currentState[y * grid.getWidth() + x] = grid.getCellState(x, y) ? 1 : 0;
        }
    }

    // Устанавливаем текущее состояние для GPU
    gpuAutomaton.SetGridState(currentState);

    // Выполняем обновление на GPU
    gpuAutomaton.Update();

    // Получаем новое состояние с GPU
    gpuAutomaton.GetGridState(nextState);

    // Обновляем сетку на основе состояния, полученного с GPU
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            bool newState = nextState[y * grid.getWidth() + x] != 0;
            Cell& cell = grid.getCell(x, y);
            bool currentState = cell.getAlive();
            cell.setAlive(newState);

            if (newState) {
                if (!currentState) {
                    // Если клетка рождается, делаем её темно-зеленого цвета
                    cell.setColor(Vector3d(0.0f, 0.5f, 0.0f));
                }
                else {
                    // Если клетка остается живой, делаем её зеленого цвета
                    cell.setColor(Vector3d(0.3f, 0.8f, 0.3f));
                }
            }
            else if (currentState) {
                // Если клетка умирает, делаем её темно-серого цвета
                cell.setColor(Vector3d(0.25f, 0.25f, 0.25f));
            }
        }
    }
}


void GameOfLife::nextGenerationCPU() {
    //saveCurrentState(); // Сохраняем текущее состояние
    int neighbors;
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            if (isToroidal) {
                neighbors = countLiveNeighbors(x, y);
            }
            else {
                neighbors = countLiveNeighborsWorld(x, y);
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