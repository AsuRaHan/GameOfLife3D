#include "GPUAutomaton.h"

GPUAutomaton::GPUAutomaton(int width, int height)
    : gridWidth(width), gridHeight(height), bufferIndex(0) {
    CreateComputeShader();
    SetupBuffers();
}

GPUAutomaton::~GPUAutomaton() {
    GL_CHECK(glDeleteBuffers(2, cellsBuffer));
}

void GPUAutomaton::CreateComputeShader() {
    const char* computedRulesShaderSource = R"(
#version 430 core

//layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(std430, binding = 0) buffer CurrentCells {
    int current[];
};

layout(std430, binding = 1) buffer NextCells {
    int next[];
};

uniform ivec2 gridSize;
uniform bool isToroidal; // ��� ����
uniform int birth; // ���������� ������� ��� ��������
uniform int survivalMin; // ����������� ���������� ������� ��� ���������
uniform int survivalMax; // ������������ ���������� ������� ��� ���������
uniform int overpopulation; // ���������� ������� ��� ������ �� �������������

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
    if (currentState == 1) { // ����� ������
        // ���������: ���-�� ������� �� survivalMin �� survivalMax ������������
        if (neighbors >= survivalMin && neighbors <= survivalMax) {
            nextState = 1;
        } else {
            nextState = 0; // ������ �� ����������� ��� �������������
        }
    } else { // ������� ������
        if (neighbors == birth) {
            nextState = 1; // ��������
        }
    }
    next[pos.y * gridSize.x + pos.x] = nextState;
}
)";

    const char* computedRulesColorsShaderSource = R"(
#version 430 core
layout(local_size_x = 32, local_size_y = 32) in;

layout(std430, binding = 0) buffer CurrentState {
    int current[];
};

layout(std430, binding = 1) buffer NextState {
    int next[];
};

layout(std430, binding = 2) buffer Colors {
    vec3 colors[];
};

uniform int gridWidth;
uniform int gridHeight;
uniform bool isToroidal;

int countLiveNeighbors(ivec2 pos) {
    int count = 0;
    for(int dy = -1; dy <= 1; ++dy) {
        for(int dx = -1; dx <= 1; ++dx) {
            if(dx == 0 && dy == 0) continue;
            ivec2 neighbor = pos + ivec2(dx, dy);
            if (isToroidal) {
                neighbor = (neighbor + ivec2(gridWidth, gridHeight)) % ivec2(gridWidth, gridHeight);
            } else {
                if (neighbor.x < 0 || neighbor.x >= gridWidth || neighbor.y < 0 || neighbor.y >= gridHeight) continue;
            }
            count += current[neighbor.y * gridWidth + neighbor.x] > 0 ? 1 : 0;
        }
    }
    return count;
}

void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= gridWidth || pos.y >= gridHeight) return;

    int currentState = current[pos.y * gridWidth + pos.x];
    int neighbors = countLiveNeighbors(pos);
    
    int nextState = 0;
    if (currentState == 1) { // ����� ������
        if (neighbors >= 2 && neighbors <= 3) {
            nextState = 1; // ������ ��������
        }
    } else { // ������� ������
        if (neighbors == 3) {
            nextState = 1; // ������ �������
        }
    }

    next[pos.y * gridWidth + pos.x] = nextState;

    // ���������� ����� ������
    if (nextState == 1) {
        colors[pos.y * gridWidth + pos.x] = vec3(0.0, 0.5, 0.0); // ������� ��� ����� ������
    } else {
        colors[pos.y * gridWidth + pos.x] = vec3(0.05, 0.05, 0.08); // ������ ��� ������� ������
    }
}
)";
    //shaderManager.loadComputeShader("computeShader", computedRulesShaderSource);
    shaderManager.loadComputeShader("computeShader", computedRulesColorsShaderSource);
    shaderManager.linkComputeProgram("computeProgram", "computeShader");
    computeProgram = shaderManager.getProgram("computeProgram");

}

void GPUAutomaton::SetupBuffers() {
    GL_CHECK(glGenBuffers(2, cellsBuffer));
    GL_CHECK(glGenBuffers(1, &colorsBuffer)); // ��������� ����� ��� ������
    // ��������, ��� colorsBuffer ������� ������
    if (colorsBuffer == 0) {
        std::cerr << "Error: Failed to generate colorsBuffer!" << std::endl;
        return;
    }
    for (int i = 0; i < 2; ++i) {
        GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[i]));
        GL_CHECK(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * gridWidth * gridHeight, nullptr, GL_DYNAMIC_COPY));
    }

    // ������������� ������ ��� ������
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorsBuffer));
    GL_CHECK(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 3 * gridWidth * gridHeight, nullptr, GL_DYNAMIC_COPY));
    std::cout << "colorsBuffer initialized with ID: " << colorsBuffer << std::endl;
}
void GPUAutomaton::SetNewGridSize(int width, int height) {
    gridWidth = width;
    gridHeight = height;
    GL_CHECK(glDeleteBuffers(2, cellsBuffer));
    SetupBuffers();
}
void GPUAutomaton::Update() {
    GL_CHECK(glUseProgram(computeProgram));
    GL_CHECK(glUniform2i(glGetUniformLocation(computeProgram, "gridSize"), gridWidth, gridHeight));
    GL_CHECK(glUniform1i(glGetUniformLocation(computeProgram, "isToroidal"), isToroidal));
    GL_CHECK(glUniform1i(glGetUniformLocation(computeProgram, "birth"), birth));
    GL_CHECK(glUniform1i(glGetUniformLocation(computeProgram, "survivalMin"), survivalMin));
    GL_CHECK(glUniform1i(glGetUniformLocation(computeProgram, "survivalMax"), survivalMax));
    GL_CHECK(glUniform1i(glGetUniformLocation(computeProgram, "overpopulation"), overpopulation));

    GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellsBuffer[bufferIndex]));
    GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellsBuffer[(bufferIndex + 1) % 2]));

    GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, colorsBuffer)); // �������� ������ ������

    GL_CHECK(glDispatchCompute((gridWidth + 31) / 32, (gridHeight + 31) / 32, 1));

    GL_CHECK(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
    bufferIndex = (bufferIndex + 1) % 2;
}


void GPUAutomaton::GetGridState(std::vector<int>& outState) {
    outState.resize(gridWidth * gridHeight);
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[bufferIndex]));
    GL_CHECK(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * gridWidth * gridHeight, outState.data()));
}

void GPUAutomaton::SetGridState(const std::vector<int>& inState) {
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[bufferIndex]));
    GL_CHECK(glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * gridWidth * gridHeight, inState.data()));
}

void GPUAutomaton::SetToroidal(bool toroidal) {
    isToroidal = toroidal;
}
