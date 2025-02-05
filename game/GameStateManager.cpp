#include "GameStateManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool GameStateManager::saveGameState(const Grid& grid, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл для записи: " << filename << std::endl;
        return false;
    }

    // Сначала записываем размеры
    file << grid.getWidth() << " " << grid.getHeight() << "\n";

    // Записываем состояния клеток
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            file << (grid.getCellState(x, y) ? "1" : "0");
        }
        file << '\n';
    }

    return true;
}

bool GameStateManager::loadGameState(Grid& grid, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл для чтения: " << filename << std::endl;
        return false;
    }

    // Читаем размеры
    int width, height;
    file >> width >> height;

    if (!validateDimensions(grid, width, height)) {
        return false;
    }

    std::string line;
    std::getline(file, line); // Пропускаем остаток строки с размерами

    // Читаем состояния клеток
    int y = 0;
    while (std::getline(file, line) && y < height) {
        for (int x = 0; x < std::min(width, (int)line.length()); ++x) {
            grid.setCellState(x, y, line[x] == '1');
        }
        ++y;
    }

    return true;
}

bool GameStateManager::saveGameStateCSV(const Grid& grid, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл для записи: " << filename << std::endl;
        return false;
    }

    // Записываем заголовок
    file << "Width:" << grid.getWidth() << ",Height:" << grid.getHeight() << "\n";
    file << "x,y,alive,type,colorR,colorG,colorB\n";

    // Записываем данные клеток
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            const Cell& cell = grid.getCell(x, y);
            const Vector3d& color = cell.getColor();
            file << x << "," << y << ","
                 << (cell.getAlive() ? "1" : "0") << ","
                 << cell.getType() << ","
                 << color.X() << ","
                 << color.Y() << ","
                 << color.Z() << "\n";
        }
    }

    return true;
}

bool GameStateManager::validateDimensions(const Grid& grid, int width, int height) {
    if (width <= 0 || height <= 0) {
        std::cerr << "Некорректные размеры в файле" << std::endl;
        return false;
    }
    if (width != grid.getWidth() || height != grid.getHeight()) {
        std::cerr << "Размеры сетки в файле не совпадают с текущими размерами" << std::endl;
        return false;
    }
    return true;
} 