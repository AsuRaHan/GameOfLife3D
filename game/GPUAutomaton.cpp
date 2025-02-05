#include "GPUAutomaton.h"

GPUAutomaton::GPUAutomaton(int width, int height)
    : gridWidth(width), gridHeight(height), bufferIndex(0) {
    CreateComputeShader();
    SetupBuffers();
}

GPUAutomaton::~GPUAutomaton() {
    glDeleteProgram(computeProgram);
    glDeleteBuffers(2, cellsBuffer);
}

void GPUAutomaton::CreateComputeShader() {
    const char* computeShaderSource = R"(
    #version 430 core

    layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

    layout(std430, binding = 0) buffer CurrentCells {
        int current[];
    };

    layout(std430, binding = 1) buffer NextCells {
        int next[];
    };

    uniform ivec2 gridSize;
    uniform bool isToroidal;

    int countLiveNeighbors(ivec2 pos) {
        int count = 0;
        for(int dy = -1; dy <= 1; ++dy) {
            for(int dx = -1; dx <= 1; ++dx) {
                if(dx == 0 && dy == 0) continue;
                ivec2 neighbor = pos + ivec2(dx, dy);
                if (isToroidal) {
                    neighbor = (neighbor + gridSize) % gridSize;
                } else {
                    if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
                }
                count += current[neighbor.y * gridSize.x + neighbor.x] > 0 ? 1 : 0;
            }
        }
        return count;
    }

    void main() {
        ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
        if (pos.x >= gridSize.x || pos.y >= gridSize.y) return;

        int currentState = current[pos.y * gridSize.x + pos.x];
        int neighbors = countLiveNeighbors(pos);
        int nextState = 0;
        if (currentState == 1) { // Живая клетка
            if (neighbors == 2 || neighbors == 3) nextState = 1;
        } else { // Мертвая клетка
            if (neighbors == 3) nextState = 1;
        }
        next[pos.y * gridSize.x + pos.x] = nextState;
    }
)";
    shaderManager.loadComputeShader("computeShader", computeShaderSource);
    shaderManager.linkComputeProgram("computeProgram", "computeShader");
    computeProgram = shaderManager.getProgram("computeProgram");

}

void GPUAutomaton::SetupBuffers() {
    glGenBuffers(2, cellsBuffer);
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[i]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * gridWidth * gridHeight, nullptr, GL_DYNAMIC_COPY);
    }
}

void GPUAutomaton::Update() {
    glUseProgram(computeProgram);
    glUniform2i(glGetUniformLocation(computeProgram, "gridSize"), gridWidth, gridHeight);
    glUniform1i(glGetUniformLocation(computeProgram, "isToroidal"), isToroidal); // Передача isToroidal

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellsBuffer[bufferIndex]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellsBuffer[(bufferIndex + 1) % 2]);

    glDispatchCompute(gridWidth, gridHeight, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    bufferIndex = (bufferIndex + 1) % 2;
}


void GPUAutomaton::GetGridState(std::vector<int>& outState) {
    outState.resize(gridWidth * gridHeight);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[bufferIndex]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * gridWidth * gridHeight, outState.data());
}

void GPUAutomaton::SetGridState(const std::vector<int>& inState) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[bufferIndex]);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * gridWidth * gridHeight, inState.data());
}

void GPUAutomaton::SetToroidal(bool toroidal) {
    isToroidal = toroidal;
}
