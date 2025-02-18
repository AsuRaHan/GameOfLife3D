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
    GPUAutomaton(const GPUAutomaton&) = delete; // ������� ����������� �����������
    GPUAutomaton& operator=(const GPUAutomaton&) = delete; // ������� �������� ������������

    ~GPUAutomaton();

    void Update();
    void GetGridState(std::vector<int>& outState);
    void SetGridState(const std::vector<int>& inState);
    void SetCellState(int x, int y, int state);
    int GetCellState(int x, int y);

    

    int birth = 3; // �� ��������� 3 ������ ��� ��������
    int survivalMin = 2; // �� ��������� ������� 2 ������ ��� ���������
    int survivalMax = 3; // �� ��������� �������� 3 ������ ��� ���������
    int overpopulation = 4; // �� ��������� 4 ��� ������ ��� ������ �� �������������
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
    void SwapBuffers(); // ����� ����� ��� ������������ �������

    ShaderManager shaderManager;
    GLuint computeProgram;
    GLuint cellsBuffer[2];
    GLint bufferIndex;

    GLuint colorsBuffer; // ����� ��� �������� ������ ������
    int currentBufferIndex; // ������ �������� ������

    int gridWidth, gridHeight;
    
};

#endif // GPU_AUTOMATON_H
