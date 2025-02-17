#pragma once
#ifndef GRID_H_
#define GRID_H_

#include "GPUAutomaton.h"

class Grid {
private:
    int width, height;
    GPUAutomaton* gpuAutomaton = nullptr; // ���� ������ ��� ���������� �� GPU
public:
    Grid(int w, int h);
    void SetGPUAutomaton(GPUAutomaton* ga) { gpuAutomaton = ga; } // ��������� ������
    void setCellColor(int x, int y, float r, float g, float b);
    void getCellColor(int x, int y, float& r, float& g, float& b);
    void setCellState(int x, int y, bool alive);
    bool getCellState(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
};
#endif // GRID_H_