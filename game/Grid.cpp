#include "Grid.h"

Grid::Grid(int w, int h) : width(w), height(h) {
    //for (auto y = 0; y < height; ++y) {
    //    cells[y].resize(width);
    //}
    cells.resize(height, std::vector<Cell>(width));
}

void Grid::setCellState(int x, int y, bool alive) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        cells[y][x].setAlive(alive);
    }
}

bool Grid::getCellState(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return cells[y][x].getAlive();
    }
    return false; // Возвращаем false для несуществующих координат
}

int Grid::getWidth() const {
    return width;
}

int Grid::getHeight() const {
    return height;
}

Cell& Grid::getCell(int x, int y) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return cells[y][x];
    }
    throw std::out_of_range("Grid::getCell: координаты за пределами сетки");
}

const Cell& Grid::getCell(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return cells[y][x];
    }
    throw std::out_of_range("Grid::getCell: координаты за пределами сетки");
}

void Grid::setCell(int x, int y, const Cell& cell) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        cells[y][x] = cell;
    }
}