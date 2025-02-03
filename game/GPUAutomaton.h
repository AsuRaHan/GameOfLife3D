#ifndef GPU_AUTOMATON_H
#define GPU_AUTOMATON_H

#include "../system/GLFunctions.h"
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

private:
    void CreateComputeShader();
    void SetupBuffers();
    void CheckShaderCompilation(GLuint shader, const std::string& name);
    void CheckProgramLinking(GLuint program);

    GLuint computeProgram;
    GLuint cellsBuffer[2];
    GLint bufferIndex;
    int gridWidth, gridHeight;
};

#endif // GPU_AUTOMATON_H
