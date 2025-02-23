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
    // Сначала вычисляем лимиты
    CheckComputeLimits();

    const char* computedRulesColorsShaderSource = R"(
#version 430 core
layout(local_size_x = {{groupSizeX}}, local_size_y = {{groupSizeY}}) in;

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

int countLiveNeighbors(ivec2 pos, int targetType) {
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
            int neighborState = current[neighbor.y * gridSize.x + neighbor.x];
            if (targetType == 0) { // Все живые
                count += (neighborState > 0) ? 1 : 0;
            } else { // Конкретный тип
                count += (neighborState == targetType) ? 1 : 0;
            }
        }
    }
    return count;
}

int determineNewType(ivec2 pos) {
    int greenNeighbors = countLiveNeighbors(pos, 1); // Зелёные
    int redNeighbors = countLiveNeighbors(pos, 2);   // Красные
    int blueNeighbors = countLiveNeighbors(pos, 3);  // Синие

    if (greenNeighbors > redNeighbors && greenNeighbors > blueNeighbors) {
        return 1; // Зелёный
    } else if (redNeighbors > greenNeighbors && redNeighbors > blueNeighbors) {
        return 2; // Красный
    } else if (blueNeighbors > greenNeighbors && blueNeighbors > redNeighbors) {
        return 3; // Синий
    } else if (greenNeighbors == redNeighbors && greenNeighbors > blueNeighbors) {
        return 4; // Жёлтый (зелёный = красный)
    } else if (greenNeighbors == blueNeighbors && greenNeighbors > redNeighbors) {
        return 5; // Оранжевый (зелёный = синий)
    } else if (redNeighbors == blueNeighbors && redNeighbors > greenNeighbors) {
        return 6; // Фиолетовый (красный = синий)
    } else { // Все три равны
        return 7; // Белый (зелёный = красный = синий)
    }
}

void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= gridSize.x || pos.y >= gridSize.y) return;

    int index = pos.y * gridSize.x + pos.x;
    int currentState = current[index];
    int neighbors = countLiveNeighbors(pos, 0); // Все живые соседи
    int nextState = currentState;

    if (currentState > 0) { // Живая клетка
        if (neighbors >= survivalMin && neighbors <= survivalMax) {
            nextState = currentState; // Сохраняем тип
        } else {
            nextState = 0; // Умирает
        }
    } else if (currentState == 0) { // Мёртвая клетка
        if (neighbors == birth) {
            nextState = determineNewType(pos); // Оживает с типом по большинству или равенству
        }
    }

    next[index] = nextState;

    // Обновляем цвета в зависимости от типа
    if (nextState == 1) { // Зелёный
        if (currentState == 1) {
            vec4 currentColor = colors[index];
            colors[index] = vec4(
                min(currentColor.r + 0.02, 0.5),
                min(currentColor.g + 0.02, 1.0),
                min(currentColor.b + 0.02, 0.5),
                1.0
            );
        } else {
            colors[index] = vec4(0.0, 0.5, 0.0, 1.0); // Начальный зелёный
        }
    } else if (nextState == 2) { // Красный
        if (currentState == 2) {
            vec4 currentColor = colors[index];
            colors[index] = vec4(
                min(currentColor.r + 0.02, 1.0),
                min(currentColor.g + 0.02, 0.5),
                min(currentColor.b + 0.02, 0.5),
                1.0
            );
        } else {
            colors[index] = vec4(0.5, 0.0, 0.0, 1.0); // Начальный красный
        }
    } else if (nextState == 3) { // Синий
        if (currentState == 3) {
            vec4 currentColor = colors[index];
            colors[index] = vec4(
                min(currentColor.r + 0.02, 0.5),
                min(currentColor.g + 0.02, 0.5),
                min(currentColor.b + 0.02, 1.0),
                1.0
            );
        } else {
            colors[index] = vec4(0.0, 0.0, 0.5, 1.0); // Начальный синий
        }
    } else if (nextState == 4) { // Жёлтый (зелёный = красный)
        if (currentState == 4) {
            vec4 currentColor = colors[index];
            colors[index] = vec4(
                min(currentColor.r + 0.02, 1.0),
                min(currentColor.g + 0.02, 1.0),
                min(currentColor.b + 0.02, 0.5),
                1.0
            );
        } else {
            colors[index] = vec4(0.5, 0.5, 0.0, 1.0); // Начальный жёлтый
        }
    } else if (nextState == 5) { // Оранжевый (зелёный = синий)
        if (currentState == 5) {
            vec4 currentColor = colors[index];
            colors[index] = vec4(
                min(currentColor.r + 0.02, 1.0),
                min(currentColor.g + 0.02, 0.5),
                min(currentColor.b + 0.02, 1.0),
                1.0
            );
        } else {
            colors[index] = vec4(0.5, 0.25, 0.5, 1.0); // Начальный оранжевый
        }
    } else if (nextState == 6) { // Фиолетовый (красный = синий)
        if (currentState == 6) {
            vec4 currentColor = colors[index];
            colors[index] = vec4(
                min(currentColor.r + 0.02, 1.0),
                min(currentColor.g + 0.02, 0.5),
                min(currentColor.b + 0.02, 1.0),
                1.0
            );
        } else {
            colors[index] = vec4(0.5, 0.0, 0.5, 1.0); // Начальный фиолетовый
        }
    } else if (nextState == 7) { // Белый (зелёный = красный = синий)
        if (currentState == 7) {
            vec4 currentColor = colors[index];
            colors[index] = vec4(
                min(currentColor.r + 0.02, 1.0),
                min(currentColor.g + 0.02, 1.0),
                min(currentColor.b + 0.02, 1.0),
                1.0
            );
        } else {
            colors[index] = vec4(0.75, 0.75, 0.75, 1.0); // Начальный белый
        }
    } else if (currentState > 0) { // Клетка умерла
        colors[index] = vec4(0.05, 0.05, 0.08, 0.0);
    }
}

)";

    // Преобразуем шаблон в std::string для замены
    std::string shaderSource = computedRulesColorsShaderSource;

    // Заменяем метки на вычисленные значения
    std::string groupSizeXStr = std::to_string(groupSizeX);
    std::string groupSizeYStr = std::to_string(groupSizeY);

    size_t pos = 0;
    while ((pos = shaderSource.find("{{groupSizeX}}", pos)) != std::string::npos) {
        shaderSource.replace(pos, 14, groupSizeXStr);
        pos += groupSizeXStr.length();
    }
    pos = 0;
    while ((pos = shaderSource.find("{{groupSizeY}}", pos)) != std::string::npos) {
        shaderSource.replace(pos, 14, groupSizeYStr);
        pos += groupSizeYStr.length();
    }

    shaderManager.loadComputeShader("computeShader", shaderSource.c_str());
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
    unsigned int state = input * 747796405u + 2891336453u;
    unsigned int word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= gridSize.x || pos.y >= gridSize.y) return;

    uint index = pos.y * gridSize.x + pos.x;
    uint random = pcg_hash(index + seed);
    float randomFloat = float(random & 0x00FFFFFF) / float(0x01000000); // [0, 1)

    if (randomFloat < density) {
        float typeFloat = randomFloat / density; // [0, 1)
        if (typeFloat < 0.142) { // 1/7
            cells[index] = 1; // Зелёный
            colors[index] = vec4(0.0, 0.6, 0.0, 1.0);
        } else if (typeFloat < 0.285) { // 2/7
            cells[index] = 2; // Красный
            colors[index] = vec4(0.5, 0.0, 0.0, 1.0);
        } else if (typeFloat < 0.428) { // 3/7
            cells[index] = 3; // Синий
            colors[index] = vec4(0.0, 0.0, 0.5, 1.0);
        } else if (typeFloat < 0.571) { // 4/7
            cells[index] = 4; // Жёлтый
            colors[index] = vec4(0.5, 0.5, 0.0, 1.0);
        } else if (typeFloat < 0.714) { // 5/7
            cells[index] = 5; // Оранжевый
            colors[index] = vec4(0.5, 0.25, 0.5, 1.0);
        } else if (typeFloat < 0.857) { // 6/7
            cells[index] = 6; // Фиолетовый
            colors[index] = vec4(0.5, 0.0, 0.5, 1.0);
        } else { // 7/7
            cells[index] = 7; // Белый
            colors[index] = vec4(0.75, 0.75, 0.75, 1.0);
        }
    } else {
        cells[index] = 0; // Мёртвая клетка
        colors[index] = vec4(0.0, 0.0, 0.0, 0.0); // Чёрный
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

    //glDispatchCompute((gridWidth + 31) / 32, (gridHeight + 31) / 32, 1);
    glDispatchCompute((gridWidth + groupSizeX - 1) / groupSizeX,
        (gridHeight + groupSizeY - 1) / groupSizeY, 1);

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

void GPUAutomaton::CheckComputeLimits() {
    GL_CHECK(glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxSharedMemorySize));
    std::cout << "Max shared memory size: " << maxSharedMemorySize << " bytes" << std::endl;

    GL_CHECK(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxWorkGroupSizeX));
    GL_CHECK(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxWorkGroupSizeY));
    std::cout << "Max work group size: (" << maxWorkGroupSizeX << ", " << maxWorkGroupSizeY << ")" << std::endl;

    GLint maxInvocations;
    GL_CHECK(glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxInvocations));
    std::cout << "Max work group invocations: " << maxInvocations << " threads" << std::endl;

    int maxElements = maxSharedMemorySize / sizeof(int);
    int maxSide = static_cast<int>(sqrt(static_cast<double>(maxElements))) - 2;

    groupSizeX = maxSide;
    if (groupSizeX > maxWorkGroupSizeX) groupSizeX = maxWorkGroupSizeX;
    if (groupSizeX > 32) groupSizeX = 32; // Максимум 32, но может уменьшиться ниже

    groupSizeY = maxSide;
    if (groupSizeY > maxWorkGroupSizeY) groupSizeY = maxWorkGroupSizeY;
    if (groupSizeY > 32) groupSizeY = 32;

    // Проверяем общее количество потоков
    int totalInvocations = groupSizeX * groupSizeY;
    if (totalInvocations > maxInvocations) {
        std::cout << "Reducing group size to fit max invocations (" << maxInvocations << ")" << std::endl;
        groupSizeX = groupSizeY = static_cast<int>(sqrt(static_cast<double>(maxInvocations)));
    }

    std::cout << "Final group size: (" << groupSizeX << ", " << groupSizeY << ")" << std::endl;
}

void GPUAutomaton::SetCellType(int x, int y, int type) {
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
        std::cerr << "SetCellType: Invalid coordinates (" << x << ", " << y << ")" << std::endl;
        return;
    }
    if (type < 0 || type > 3) {
        std::cerr << "SetCellType: Invalid type value (" << type << "). Use 0 (dead), 1 (green), 2 (red), 3 (blue)." << std::endl;
        return;
    }

    int index = y * gridWidth + x;

    // Устанавливаем тип в cellsBuffer
    int stateData = type;
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellsBuffer[currentBufferIndex]));
    GL_CHECK(glBufferSubData(GL_SHADER_STORAGE_BUFFER, index * sizeof(int), sizeof(int), &stateData));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    // Устанавливаем цвет в colorsBuffer
    float colorData[4];
    if (type == 1) { // Зелёный
        colorData[0] = 0.0f; colorData[1] = 0.5f; colorData[2] = 0.0f; colorData[3] = 1.0f;
    }
    else if (type == 2) { // Красный
        colorData[0] = 0.5f; colorData[1] = 0.0f; colorData[2] = 0.0f; colorData[3] = 1.0f;
    }
    else if (type == 3) { // Синий
        colorData[0] = 0.0f; colorData[1] = 0.0f; colorData[2] = 0.5f; colorData[3] = 1.0f;
    }
    else { // Мёртвая
        colorData[0] = 0.05f; colorData[1] = 0.05f; colorData[2] = 0.08f; colorData[3] = 0.0f;
    }

    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorsBuffer));
    GL_CHECK(glBufferSubData(GL_SHADER_STORAGE_BUFFER, index * sizeof(float) * 4, sizeof(float) * 4, colorData));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}