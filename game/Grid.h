#pragma once

#include <vector>
#include "Cell.h"

class Grid {
private:
    std::vector<std::vector<Cell>> cells;
    int width, height;

public:
    Grid(int w, int h);
    void setCellState(int x, int y, bool alive);
    bool getCellState(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
};