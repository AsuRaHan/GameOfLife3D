#include "GPUAutomaton.h"

GPUAutomaton::GPUAutomaton(int width, int height)
    : gridWidth(width), gridHeight(height), bufferIndex(0) {
    CreateComputeShader();
    SetupBuffers();
}

GPUAutomaton::~GPUAutomaton() {
    GL_CHECK(glDeleteBuffers(2, cellsBuffer));
    GL_CHECK(glDeleteBuffers(1, &colorsBuffer));
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
uniform bool isToroidal; // тип мира
uniform int birth; // Количество соседей для рождения
uniform int survivalMin; // Минимальное количество соседей для выживания
uniform int survivalMax; // Максимальное количество соседей для выживания
uniform int overpopulation; // Количество соседей для смерти от перенаселения

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
        // Выживание: кол-во соседей от survivalMin до survivalMax включительно
        if (neighbors >= survivalMin && neighbors <= survivalMax) {
            nextState = 1;
        } else {
            nextState = 0; // Смерть от одиночества или перенаселения
        }
    } else { // Мертвая клетка
        if (neighbors == birth) {
            nextState = 1; // Рождение
        }
    }
    next[pos.y * gridSize.x + pos.x] = nextState;
}
)";

    const char* computedRulesColorsShaderSource = R"(
    #version 430 core
    layout(local_size_x = 32, local_size_y = 32) in;

    layout(std430, binding = 0) buffer CurrentCells {
        int current[];
    };

    layout(std430, binding = 1) buffer NextCells {
        int next[];
    };

    layout(std430, binding = 2) buffer Colors {
        vec4 colors[];
    };

    uniform ivec2 gridSize;
    uniform bool isToroidal;
    uniform int birth;
    uniform int survivalMin;
    uniform int survivalMax;
    uniform int overpopulation;

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

        int index = pos.y * gridSize.x + pos.x;
        int currentState = current[index];
        int neighbors = countLiveNeighbors(pos);
        
        int nextState = 0;
        if (currentState == 1) {
            if (neighbors >= survivalMin && neighbors <= survivalMax) {
                nextState = 1;
            }
        } else {
            if (neighbors == birth) {
                nextState = 1;
            }
        }

        next[index] = nextState;

        if (nextState == 1) {
            colors[index] = vec4(0.0, 0.5, 0.0, 1.0);
        } else {
            colors[index] = vec4(0.05, 0.05, 0.08, 1.0);
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
    GL_CHECK(glGenBuffers(1, &colorsBuffer));

    for (int i = 0; i < 2; ++i) {
        GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[i]));
        GL_CHECK(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * gridWidth * gridHeight, nullptr, GL_DYNAMIC_COPY));
    }

    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorsBuffer));
    GL_CHECK(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * gridWidth * gridHeight, nullptr, GL_DYNAMIC_COPY));
}

void GPUAutomaton::SetNewGridSize(int width, int height) {
    gridWidth = width;
    gridHeight = height;
    GL_CHECK(glDeleteBuffers(2, cellsBuffer));
    GL_CHECK(glDeleteBuffers(1, &colorsBuffer));
    SetupBuffers();
}


void GPUAutomaton::Update() {
    glUseProgram(computeProgram);

    glUniform2i(glGetUniformLocation(computeProgram, "gridSize"), gridWidth, gridHeight);
    glUniform1i(glGetUniformLocation(computeProgram, "isToroidal"), isToroidal);
    glUniform1i(glGetUniformLocation(computeProgram, "birth"), birth);
    glUniform1i(glGetUniformLocation(computeProgram, "survivalMin"), survivalMin);
    glUniform1i(glGetUniformLocation(computeProgram, "survivalMax"), survivalMax);
    glUniform1i(glGetUniformLocation(computeProgram, "overpopulation"), overpopulation);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellsBuffer[currentBufferIndex]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellsBuffer[(currentBufferIndex + 1) % 2]);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, colorsBuffer);

    glDispatchCompute((gridWidth + 31) / 32, (gridHeight + 31) / 32, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    //bufferIndex = (bufferIndex + 1) % 2;

    SwapBuffers();
}
//  Как это работает
//  На каждом шаге Update() :
//      cellsBuffer[currentBufferIndex] привязывается к binding = 0 как входной буфер(current).
//      cellsBuffer[(currentBufferIndex + 1) % 2] привязывается к binding = 1 как выходной буфер(next).
//      Шейдер выполняет вычисления, записывая новое состояние в next и цвета в colorsBuffer.
//      После выполнения SwapBuffers() переключает currentBufferIndex, делая next новым current для следующего шага.
void GPUAutomaton::SwapBuffers() {
    currentBufferIndex = (currentBufferIndex + 1) % 2;
}

void GPUAutomaton::SetGridState(const std::vector<int>& inState) {
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[currentBufferIndex]));
    GL_CHECK(glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * gridWidth * gridHeight, inState.data()));
}

void GPUAutomaton::GetGridState(std::vector<int>& outState) {
    outState.resize(gridWidth * gridHeight);
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[currentBufferIndex]));
    GL_CHECK(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * gridWidth * gridHeight, outState.data()));
}


void GPUAutomaton::SetToroidal(bool toroidal) {
    isToroidal = toroidal;
}

void GPUAutomaton::SetCellColor(int x,int y, float r, float g, float b) {
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
        std::cerr << "SetCellColor: Invalid coordinates (" << x << ", " << y << ")" << std::endl;
        return;
    }
    if (colorsBuffer == 0) {
        std::cerr << "SetCellColor: colorsBuffer is not initialized!" << std::endl;
        return;
    }
    // Вычисляем индекс в буфере
    int index = y * gridWidth + x;

    // Подготавливаем данные для записи (vec4)
    float colorData[4] = { r, g, b, 1.0f };

    // Привязываем colorsBuffer и записываем данные
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorsBuffer));
    GL_CHECK(glBufferSubData(GL_SHADER_STORAGE_BUFFER,
        index * sizeof(float) * 4, // Смещение в байтах
        sizeof(float) * 4,         // Размер данных (4 float = vec4)
        colorData));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

void GPUAutomaton::GetCellColor(int x, int y, float& r, float& g, float& b) {
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
        std::cerr << "GetCellColor: Invalid coordinates (" << x << ", " << y << ")" << std::endl;
        r = g = b = 0.0f; // Возвращаем черный цвет с нулевой прозрачностью в случае ошибки
        return;
    }
    if (colorsBuffer == 0) {
        std::cerr << "GetCellColor: colorsBuffer is not initialized!" << std::endl;
        r = g = b = 0.0f;
        return;
    }

    int index = y * gridWidth + x;
    float colorData[4];

    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorsBuffer));
    GL_CHECK(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,
        index * sizeof(float) * 4, // Смещение в байтах
        sizeof(float) * 4,         // Размер данных (vec4)
        colorData));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    r = colorData[0];
    g = colorData[1];
    b = colorData[2];
    //a = colorData[3];
}

void GPUAutomaton::SetCellState(int x, int y, int state) {
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
        std::cerr << "SetCellState: Invalid coordinates (" << x << ", " << y << ")" << std::endl;
        return;
    }
    if (state != 0 && state != 1) {
        std::cerr << "SetCellState: Invalid state value (" << state << "). Use 0 (dead) or 1 (alive)." << std::endl;
        return;
    }

    int index = y * gridWidth + x;
    int stateData = state;

    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[currentBufferIndex]));
    GL_CHECK(glBufferSubData(GL_SHADER_STORAGE_BUFFER,
        index * sizeof(int), // Смещение в байтах
        sizeof(int),         // Размер данных (1 int)
        &stateData));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

int GPUAutomaton::GetCellState(int x, int y) {
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
        std::cerr << "GetCellState: Invalid coordinates (" << x << ", " << y << ")" << std::endl;
        return 0; // Возвращаем мертвое состояние в случае ошибки
    }

    int index = y * gridWidth + x;
    int stateData;

    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[currentBufferIndex]));
    GL_CHECK(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,
        index * sizeof(int), // Смещение в байтах
        sizeof(int),         // Размер данных (1 int)
        &stateData));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    return stateData;
}