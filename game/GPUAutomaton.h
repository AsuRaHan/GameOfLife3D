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
    //GPUAutomaton(int width, int height);
    //GPUAutomaton(const GPUAutomaton&) = delete; // Удаляем конструктор копирования
    //GPUAutomaton& operator=(const GPUAutomaton&) = delete; // Удаляем оператор присваивания

    //~GPUAutomaton();
    GPUAutomaton(int width, int height);
    GPUAutomaton(const GPUAutomaton&) = delete;
    GPUAutomaton& operator=(const GPUAutomaton&) = delete;

    virtual ~GPUAutomaton(); // Добавляем virtual для корректного наследования

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
    int neighborhoodRadius = 1; // радиус окрестности Мура по умолчанию 1
    void SetBirth(int b) { birth = b; }
    void SetSurvivalMin(int smin) { survivalMin = smin; }
    void SetSurvivalMax(int smax) { survivalMax = smax; }
    void SetOverpopulation(int o) { overpopulation = o; }
    void SetToroidal(bool toroidal);
    void SetNeighborhoodRadius(int radius) {
        neighborhoodRadius = radius;
    };

    void SetNewGridSize(int width, int height);

    GLint getColorsBuffer() { return colorsBuffer; };

    void SetCellColor(int x, int y, float r, float g, float b);
    void GetCellColor(int x, int y, float& r, float& g, float& b);

    void SetColorsBuf(const std::vector<float>& colors);
    void GetColorsBuf(std::vector<float>& colors);

    void ClearGrid(); // метод для очистки
    void RandomizeGrid(float density, unsigned int seed); // метод для рандомизации

    void SetCellType(int x, int y, int type); // метод для установки типа клетки
    // ========== Другие правила игры устанавливаются тут
    void setBirthRules(bool rules[9]) {
        for (int i = 0; i < 9; i++) birthRules[i] = rules[i] ? 1 : 0;
        glUseProgram(computeProgram);
        glUniform1iv(glGetUniformLocation(computeProgram, "birthCounts"), 9, birthRules);
    }
    void setSurviveRules(bool rules[9]) {
        for (int i = 0; i < 9; i++) surviveRules[i] = rules[i] ? 1 : 0;
        glUseProgram(computeProgram);
        glUniform1iv(glGetUniformLocation(computeProgram, "surviveCounts"), 9, surviveRules);
    }
    void setUseAdvancedRules(bool enabled) {
        useAdvancedRules = enabled ? 1 : 0;
        glUseProgram(computeProgram);
        glUniform1i(glGetUniformLocation(computeProgram, "useAdvancedRules"), useAdvancedRules);
    }
    void setOverpopulationRules(bool rules[9]) {
        for (int i = 0; i < 9; i++) overpopulationRules[i] = rules[i] ? 1 : 0;
        glUseProgram(computeProgram);
        glUniform1iv(glGetUniformLocation(computeProgram, "overpopulationCounts"), 9, overpopulationRules);
    }
protected:
    virtual void CreateComputeShader();
    void SetupBuffers();
    void SwapGridBuffers(); // метод для переключения буферов

    ShaderManager shaderManager;
    GLuint computeProgram;
    GLuint cellsBuffer[2];
    GLint bufferIndex;

    GLuint colorsBuffer; // Буфер для хранения цветов клеток
    int currentBufferIndex; // Индекс текущего буфера

    int gridWidth, gridHeight;
    
    GLuint clearProgram; // свойство для программы очистки
    void LoadClearShader(); // приватный метод для загрузки шейдера очистки

    GLuint randomizeProgram; // свойство для программы рандомизации
    void LoadRandomizeShader(); // метод для загрузки шейдера рандомизации

    // Новый метод для проверки лимитов
    void CheckComputeLimits();
    // Переменные для лимитов видеокарты
    GLint maxSharedMemorySize;      // Максимальный размер общей памяти (в байтах)
    GLint maxWorkGroupSizeX;        // Максимальный размер группы по X
    GLint maxWorkGroupSizeY;        // Максимальный размер группы по Y
    GLint groupSizeX;               // Текущий размер группы по X
    GLint groupSizeY;               // Текущий размер группы по Y
    //============ Другие правила игры
    int birthRules[9];  // 0 или 1 для каждого числа соседей
    int surviveRules[9];
    int useAdvancedRules; // 0 - стандартный, 1 - расширенный ==== режимы правил
    int overpopulationRules[9];
};

#endif // GPU_AUTOMATON_H
