#include "GPUAutomaton.h"

GPUAutomaton::GPUAutomaton(int width, int height)
    : gridWidth(width), gridHeight(height), bufferIndex(0) {
    CreateComputeShader();
    LoadClearShader();
    LoadRandomizeShader();
    SetupBuffers();
}

GPUAutomaton::~GPUAutomaton() {
    GL_CHECK(glDeleteBuffers(2, cellsBuffer));
    GL_CHECK(glDeleteBuffers(1, &colorsBuffer));
}

void GPUAutomaton::CreateComputeShader() {
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

    // Если клетка выжила, делаем её цвет светлее
    if (nextState == 1) {
        if (currentState == 1) {  // Если клетка была жива и остается живой
            vec4 currentColor = colors[index];
            // Увеличиваем яркость зеленого цвета, но ограничиваем максимум
            colors[index] = vec4(
                min(currentColor.r + 0.02, 0.5),  // Увеличиваем красный канал
                min(currentColor.g + 0.02, 1.0),  // Здесь увеличиваем зелёный канал
                min(currentColor.b + 0.02, 0.5),  // Увеличиваем синий канал
                1.0
            );
        } else {
            // Если клетка только что ожила, устанавливаем базовый цвет
            colors[index] = vec4(0.0, 0.5, 0.0, 1.0);  // Начальный зеленый цвет
        }
    } else if(currentState == 1) {
        // Если клетка мертва, устанавливаем цвет мертвой клетки
        colors[index] = vec4(0.05, 0.05, 0.08, 0.0);
    }
}



)";
    shaderManager.loadComputeShader("computeShader", computedRulesColorsShaderSource);
    shaderManager.linkComputeProgram("computeProgram", "computeShader");
    computeProgram = shaderManager.getProgram("computeProgram");

}

void GPUAutomaton::LoadClearShader() {
    const char* clearShaderSource = R"(
    #version 430 core
    layout(local_size_x = 32, local_size_y = 32) in;
    
    layout(std430, binding = 0) buffer CellBuffer {
        int cells[];
    };
    
    layout(std430, binding = 1) buffer ColorBuffer {
        vec4 colors[];
    };
    
    uniform ivec2 gridSize;

    void main() {
        ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
        if (pos.x >= gridSize.x || pos.y >= gridSize.y) return;
        
        uint index = pos.y * gridSize.x + pos.x;
        cells[index] = 0; // Мертвая клетка
        colors[index] = vec4(0.0, 0.0, 0.0, 0.0); // Черный цвет
    }
    )";

    shaderManager.loadComputeShader("clearShader", clearShaderSource);
    shaderManager.linkComputeProgram("clearProgram", "clearShader");
    clearProgram = shaderManager.getProgram("clearProgram");
}

void GPUAutomaton::LoadRandomizeShader() {
    const char* randomizeShaderSource = R"(
    #version 430 core
    layout(local_size_x = 32, local_size_y = 32) in;
    
    layout(std430, binding = 0) buffer CellBuffer {
        int cells[];
    };
    
    layout(std430, binding = 1) buffer ColorBuffer {
        vec4 colors[];
    };
    
    uniform ivec2 gridSize;
    uniform float density;
    uniform unsigned int seed;

unsigned int pcg_hash(unsigned int input) {
    unsigned int state = input * 747796405u + 2891336453u; // Используйте 'u' для литералов беззнаковых чисел
    unsigned int word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

    void main() {
        ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
        if (pos.x >= gridSize.x || pos.y >= gridSize.y) return;
        
        uint index = pos.y * gridSize.x + pos.x;
        uint random = pcg_hash(index + seed);
        float randomFloat = float(random & 0x00FFFFFF) / float(0x01000000); // Преобразование в float [0, 1)
        
        if (randomFloat < density) {
            cells[index] = 1; // Живая клетка
            colors[index] = vec4(0.0, 0.6, 0.0, 1.0); // Зеленый цвет
        } else {
            cells[index] = 0; // Мертвая клетка
            colors[index] = vec4(0.0, 0.0, 0.0, 0.0); // Черный цвет
        }
    }
    )";

    shaderManager.loadComputeShader("randomizeShader", randomizeShaderSource);
    shaderManager.linkComputeProgram("randomizeProgram", "randomizeShader");
    randomizeProgram = shaderManager.getProgram("randomizeProgram");
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
    ClearGrid();
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

    SwapBuffers();
}

/**
* Как это работает
* На каждом шаге Update() :
*      cellsBuffer[currentBufferIndex] привязывается к binding = 0 как входной буфер(current).
*      cellsBuffer[(currentBufferIndex + 1) % 2] привязывается к binding = 1 как выходной буфер(next).
*      Шейдер выполняет вычисления, записывая новое состояние в next и цвета в colorsBuffer.
*      После выполнения SwapBuffers() переключает currentBufferIndex, делая next новым current для следующего шага
*/
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

void GPUAutomaton::ClearGrid() {
    glUseProgram(clearProgram);

    glUniform2i(glGetUniformLocation(clearProgram, "gridSize"), gridWidth, gridHeight);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellsBuffer[currentBufferIndex]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, colorsBuffer);

    glDispatchCompute((gridWidth + 31) / 32, (gridHeight + 31) / 32, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GPUAutomaton::RandomizeGrid(float density, unsigned int seed) {
    glUseProgram(randomizeProgram);

    glUniform2i(glGetUniformLocation(randomizeProgram, "gridSize"), gridWidth, gridHeight);
    glUniform1f(glGetUniformLocation(randomizeProgram, "density"), density);
    glUniform1ui(glGetUniformLocation(randomizeProgram, "seed"), seed);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellsBuffer[currentBufferIndex]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, colorsBuffer);

    glDispatchCompute((gridWidth + 31) / 32, (gridHeight + 31) / 32, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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
    //glFinish();
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
    //glFinish();
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[currentBufferIndex]));
    GL_CHECK(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,
        index * sizeof(int), // Смещение в байтах
        sizeof(int),         // Размер данных (1 int)
        &stateData));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    return stateData;
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
    float colorData[4] = { r, g, b, 0.0f };
    //glFinish();
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
    //glFinish();
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

void GPUAutomaton::SetColorsBuf(const std::vector<float>& colors) {
    if (colors.size() != gridWidth * gridHeight * 4) {
        std::cerr << "SetColorsBuf: Invalid color data size!" << std::endl;
        return;
    }
    if (colorsBuffer == 0) {
        std::cerr << "SetColorsBuf: colorsBuffer is not initialized!" << std::endl;
        return;
    }
    //glFinish();
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorsBuffer));
    GL_CHECK(glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * colors.size(), colors.data()));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

void GPUAutomaton::GetColorsBuf(std::vector<float>& colors) {
    colors.resize(gridWidth * gridHeight * 4);
    if (colorsBuffer == 0) {
        std::cerr << "GetColorsBuf: colorsBuffer is not initialized!" << std::endl;
        return;
    }
    //glFinish();
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorsBuffer));
    GL_CHECK(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * colors.size(), colors.data()));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}