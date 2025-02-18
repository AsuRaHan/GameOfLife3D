#pragma once
#ifndef GRID_H_
#define GRID_H_

#include "GPUAutomaton.h"

class Grid {
private:
    int width, height;
    GPUAutomaton* gpuAutomaton = nullptr; // член класса для вычислений на GPU
public:
    Grid(int w, int h);
    void SetGPUAutomaton(GPUAutomaton* ga) { gpuAutomaton = ga; } // Добавляем сеттер
    //GPUAutomaton GetGPUAutomaton() const { return *gpuAutomaton; } // Добавляем геттер
    GPUAutomaton* GetGPUAutomaton() const { return gpuAutomaton; }

    void setCellColor(int x, int y, float r, float g, float b);
    void getCellColor(int x, int y, float& r, float& g, float& b);
    void setCellState(int x, int y, bool alive);
    bool getCellState(int x, int y) const;
    int getWidth() const;
    void setWidth(int w) { width = w; };
    int getHeight() const;
    void setHeight(int h) { height = h; };
    void setSize(int w, int h);
};
#endif // GRID_H_