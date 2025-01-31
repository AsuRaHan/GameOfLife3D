#include "GameOfLife.h"

//#include <cstdlib> // Для rand() и srand()
//#include <ctime>   // Для time()

GameOfLife::GameOfLife(Grid& g) : grid(g) {
    //std::srand(static_cast<unsigned int>(std::time(nullptr))); // Инициализация генератора случайных чисел
}

//int count = 0;
//for (int i = -1; i <= 1; ++i) {
//    for (int j = -1; j <= 1; ++j) {
//        if (i == 0 && j == 0) continue; // Пропускаем саму клетку
//        int nx = x + i, ny = y + j;
//        if (nx >= 0 && nx < grid.getWidth() && ny >= 0 && ny < grid.getHeight() && grid.getCellState(nx, ny)) {
//            count++;
//        }
//    }
//}
//return count;
int GameOfLife::countLiveNeighbors(int x, int y) const {
    int count = 0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue; // Пропускаем саму клетку

            // Используем модуль для тороидальной модели
            int nx = (x + i + grid.getWidth()) % grid.getWidth();
            int ny = (y + j + grid.getHeight()) % grid.getHeight();

            if (grid.getCellState(nx, ny)) {
                count++;
            }
        }
    }
    return count;
}

void GameOfLife::nextGeneration() {
    saveCurrentState(); // Сохраняем текущее состояние перед переходом к следующему поколению
    Grid newGrid(grid.getWidth(), grid.getHeight()); // Создаем новую сетку для следующего поколения
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            int neighbors = countLiveNeighbors(x, y);
            bool currentState = grid.getCellState(x, y);
            if (currentState) {
                newGrid.setCellState(x, y, neighbors == 2 || neighbors == 3); // Правило для живых клеток
            }
            else {
                newGrid.setCellState(x, y, neighbors == 3); // Правило для мертвых клеток
            }
        }
    }
    grid = newGrid; // Обновляем текущую сетку
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