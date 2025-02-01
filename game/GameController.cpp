#include "GameController.h"
#include <random> // Для генерации случайных чисел


GameController::GameController(int width, int height, float cellSize)
    : grid(width, height), gameOfLife(grid), cellSize(cellSize), isRunning(false) {
}

void GameController::initializeGrid() {
    // Устанавливаем случайные живые клетки для демонстрации
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            // Создаем простой узор для тестирования
            if ((x == 40 && y == 30) ||  // Одиночная клетка
                (x == 41 && y == 30) ||  // Соседняя клетка
                (x == 42 && y == 30) ||  // Ещё одна соседняя клетка
                (x == 41 && y == 31)) {  // Клетка снизу для формирования "блока"
                grid.setCellState(x, y, true);
            }
        }
    }
}

void GameController::randomizeGrid(float density) {
    // Генератор случайных чисел
    std::random_device rd;  // Только для инициализации генератора
    std::mt19937 gen(rd()); // Стандартный мерсенновский твистер
    std::uniform_real_distribution<> dis(0.0, 1.0); // Равномерное распределение

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            // Если случайное число меньше density, клетка становится живой
            if (dis(gen) < density) {
                grid.setCellState(x, y, true);
            }
            else {
                grid.setCellState(x, y, false);
            }
            //float r = static_cast<float>(rand()) / RAND_MAX;
            //float g = static_cast<float>(rand()) / RAND_MAX;
            //float b = static_cast<float>(rand()) / RAND_MAX;
            grid.getCell(x, y).setColor(Vector3d(0.0f, 0.6f, 0.0f));
        }
    }
}

void GameController::update() {
    if (isRunning) {
        gameOfLife.nextGeneration();
    }
}

void GameController::startSimulation() {
    isRunning = true;
}

void GameController::stopSimulation() {
    isRunning = false;
}

void GameController::stepSimulation() {
    if (!isRunning) {
        gameOfLife.nextGeneration();
    }
}

void GameController::previousGeneration() {
    if (!isRunning) {
        gameOfLife.previousGeneration();
    }
}
bool GameController::isSimulationRunning() const {
    return isRunning;
}

void GameController::toggleCellState(int x, int y) {
    bool currentState = grid.getCellState(x, y);
    grid.setCellState(x, y, !currentState);
}

bool GameController::getCellState(int x, int y) const {
    return grid.getCellState(x, y);
}