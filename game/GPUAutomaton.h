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

    int birth = 3; // �� ��������� 3 ������ ��� ��������
    int survivalMin = 2; // �� ��������� ������� 2 ������ ��� ���������
    int survivalMax = 3; // �� ��������� �������� 3 ������ ��� ���������
    int overpopulation = 4; // �� ��������� 4 ��� ������ ��� ������ �� �������������
    void SetBirth(int b) { birth = b; }
    void SetSurvivalMin(int smin) { survivalMin = smin; }
    void SetSurvivalMax(int smax) { survivalMax = smax; }
    void SetOverpopulation(int o) { overpopulation = o; }
    
    void SetNewGridSize(int width, int height);

    GLint getColorsBuffer() { return colorsBuffer; };

    void SetCellColor(int x, int y, float r, float g, float b);
    void GetCellColor(int x, int y, float& r, float& g, float& b);

    void SetCellState(int x, int y, int state);
    int GetCellState(int x, int y);

private:
    void CreateComputeShader();
    void SetupBuffers();
    void SwapBuffers(); // ����� ����� ��� ������������ �������

    ShaderManager shaderManager;
    GLuint computeProgram;
    GLuint cellsBuffer[2];
    GLint bufferIndex;

    GLuint colorsBuffer; // ����� ��� �������� ������ ������
    int currentBufferIndex; // ������ �������� ������

    int gridWidth, gridHeight;
    bool isToroidal;
};

#endif // GPU_AUTOMATON_H
