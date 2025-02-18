#pragma once
#ifndef GPU_AUTOMATON_H
#define GPU_AUTOMATON_H

#include "../system/GLFunctions.h"
#include "../system/ShaderManager.h"
#include <vector>
#include <string>
#include <iostream>

class GPUAutomaton {
public:
    GPUAutomaton(int width, int height);
    GPUAutomaton(const GPUAutomaton&) = delete; // Удаляем конструктор копирования
    GPUAutomaton& operator=(const GPUAutomaton&) = delete; // Удаляем оператор присваивания

    ~GPUAutomaton();

    void Update();
    void GetGridState(std::vector<int>& outState);
    void SetGridState(const std::vector<int>& inState);
    void SetCellState(int x, int y, int state);
    int GetCellState(int x, int y);

    

    int birth = 3; // По умолчанию 3 соседа для рождения
    int survivalMin = 2; // По умолчанию минимум 2 соседа для выживания
    int survivalMax = 3; // По умолчанию максимум 3 соседа для выживания
    int overpopulation = 4; // По умолчанию 4 или больше для смерти от перенаселения
    bool isToroidal = true;
    void SetBirth(int b) { birth = b; }
    void SetSurvivalMin(int smin) { survivalMin = smin; }
    void SetSurvivalMax(int smax) { survivalMax = smax; }
    void SetOverpopulation(int o) { overpopulation = o; }
    void SetToroidal(bool toroidal);

    void SetNewGridSize(int width, int height);

    GLint getColorsBuffer() { return colorsBuffer; };

    void SetCellColor(int x, int y, float r, float g, float b);
    void GetCellColor(int x, int y, float& r, float& g, float& b);

    void SetColorsBuf(const std::vector<float>& colors);
    void GetColorsBuf(std::vector<float>& colors);


private:
    void CreateComputeShader();
    void SetupBuffers();
    void SwapBuffers(); // Новый метод для переключения буферов

    ShaderManager shaderManager;
    GLuint computeProgram;
    GLuint cellsBuffer[2];
    GLint bufferIndex;

    GLuint colorsBuffer; // Буфер для хранения цветов клеток
    int currentBufferIndex; // Индекс текущего буфера

    int gridWidth, gridHeight;
    
};

#endif // GPU_AUTOMATON_H
