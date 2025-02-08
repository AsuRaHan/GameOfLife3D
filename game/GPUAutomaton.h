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
    ~GPUAutomaton();

    void Update();
    void GetGridState(std::vector<int>& outState);
    void SetGridState(const std::vector<int>& inState);
    void SetToroidal(bool toroidal);

    int birth = 3; // По умолчанию 3 соседа для рождения
    int survivalMin = 2; // По умолчанию минимум 2 соседа для выживания
    int survivalMax = 3; // По умолчанию максимум 3 соседа для выживания
    int overpopulation = 4; // По умолчанию 4 или больше для смерти от перенаселения
    void SetBirth(int b) { birth = b; }
    void SetSurvivalMin(int smin) { survivalMin = smin; }
    void SetSurvivalMax(int smax) { survivalMax = smax; }
    void SetOverpopulation(int o) { overpopulation = o; }

private:
    void CreateComputeShader();
    void SetupBuffers();

    ShaderManager shaderManager;
    GLuint computeProgram;
    GLuint cellsBuffer[2];
    GLint bufferIndex;

    int gridWidth, gridHeight;
    bool isToroidal;
};

#endif // GPU_AUTOMATON_H
