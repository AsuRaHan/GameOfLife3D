#include "Grid.h"

Grid::Grid(int w, int h) : width(w), height(h) {
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
    return false; // ¬озвращаем false дл€ несуществующих координат
}

int Grid::getWidth() const {
    return width;
}

int Grid::getHeight() const {
    return height;
}