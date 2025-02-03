#include "GPUAutomaton.h"

GPUAutomaton::GPUAutomaton(int width, int height)
    : gridWidth(width), gridHeight(height), gridSize(width* height), bufferIndex(0) {
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

        int countLiveNeighbors(ivec2 pos) {
            int count = 0;
            for(int dy = -1; dy <= 1; ++dy) {
                for(int dx = -1; dx <= 1; ++dx) {
                    if(dx == 0 && dy == 0) continue;
                    ivec2 neighbor = (pos + ivec2(dx, dy) + gridSize) % gridSize;
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

    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderSource, NULL);
    glCompileShader(computeShader);
    CheckShaderCompilation(computeShader, "Compute Shader");

    computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    CheckProgramLinking(computeProgram);
    glDeleteShader(computeShader);
}

void GPUAutomaton::SetupBuffers() {
    glGenBuffers(2, cellsBuffer);
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[i]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * gridSize, nullptr, GL_DYNAMIC_COPY);
    }
}

void GPUAutomaton::DispatchCompute() {
    glUseProgram(computeProgram);
    glUniform2i(glGetUniformLocation(computeProgram, "gridSize"), gridWidth, gridHeight);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellsBuffer[bufferIndex]); // Используем текущий буфер
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellsBuffer[(bufferIndex + 1) % 2]); // Следующий буфер для обновления

    glDispatchCompute(gridWidth, gridHeight, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Переключаем буферы для следующего цикла
    bufferIndex = (bufferIndex + 1) % 2;
}

void GPUAutomaton::Update() {
    // Смена буферов для двойной буферизации
    int currentBuffer = bufferIndex;
    bufferIndex = (bufferIndex + 1) % 2;

    glUseProgram(computeProgram);
    glUniform2i(glGetUniformLocation(computeProgram, "gridSize"), gridWidth, gridHeight);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellsBuffer[currentBuffer]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellsBuffer[bufferIndex]);

    glDispatchCompute(gridWidth, gridHeight, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GPUAutomaton::GetGridState(std::vector<int>& outState) {
    outState.resize(gridSize);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[bufferIndex]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * gridSize, outState.data());
}

void GPUAutomaton::SetGridState(const std::vector<int>& inState) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[bufferIndex]);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * gridSize, inState.data());
}


void GPUAutomaton::CheckShaderCompilation(GLuint shader, const std::string& name) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::" << name << "::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void GPUAutomaton::CheckProgramLinking(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}