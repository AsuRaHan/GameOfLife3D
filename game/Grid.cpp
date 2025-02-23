#include "Grid.h"

Grid::Grid(int w, int h) : width(w), height(h) {
    
}

void Grid::setCellColor(int x, int y, float r, float g, float b)
{
    if (!gpuAutomaton) return;
    gpuAutomaton->SetCellColor( x,  y,  r,  g,  b);
}

void Grid::getCellColor(int x, int y, float& r, float& g, float& b)
{
    if (!gpuAutomaton) return;
    gpuAutomaton->GetCellColor( x,  y, r, g, b);
}

void Grid::setCellState(int x, int y, int alive) {
    if (!gpuAutomaton) return;
    if (x >= 0 && x < width && y >= 0 && y < height) {
        gpuAutomaton->SetCellState(x, y, alive);
    }
}

int Grid::getCellState(int x, int y) const {
    if (!gpuAutomaton) return false;
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return gpuAutomaton->GetCellState(x, y);
    }
    return 0; // Возвращаем false для несуществующих координат
}

int Grid::getWidth() const {
    return width;
}

int Grid::getHeight() const {
    return height;
}

void Grid::setSize(int w, int h)
{
    width = w;
    height = h;
}

void Grid::SetCellType(int x, int y, int type) {
    if (!gpuAutomaton) return;
    if (x >= 0 && x < width && y >= 0 && y < height) {
        gpuAutomaton->SetCellType(x, y, type);
    }
}