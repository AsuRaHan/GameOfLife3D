#include "GPUAutomaton.h"

GPUAutomaton::GPUAutomaton(int width, int height)
    : gridWidth(width), gridHeight(height), bufferIndex(0) {
    CreateComputeShader();
    LoadClearShader();
    LoadRandomizeShader();
    SetupBuffers();
}

GPUAutomaton::~GPUAutomaton() {
    if (cellsBuffer[0] && cellsBuffer[1]) {
        GL_CHECK(glDeleteBuffers(2, cellsBuffer));
    }
    if (colorsBuffer) {
        GL_CHECK(glDeleteBuffers(1, &colorsBuffer));
    }
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
uniform int neighborhoodRadius;
uniform bool isToroidal;

uniform int birth;
uniform int survivalMin;
uniform int survivalMax;
uniform int overpopulation;

uniform int birthCounts[9];
uniform int surviveCounts[9];
uniform int useAdvancedRules; // 0 - стадартный режим, 1 - расширенный
uniform int overpopulationCounts[9];

bool canBeBorn(int neighbors) {
    return neighbors >= 0 && neighbors <= 8 && birthCounts[neighbors] == 1;
}

bool canSurvive(int neighbors) {
    return neighbors >= 0 && neighbors <= 8 && surviveCounts[neighbors] == 1;
}

bool isOverpopulated(int neighbors) {
    return neighbors >= 0 && neighbors <= 8 && overpopulationCounts[neighbors] == 1;
}

int countLiveNeighbors(ivec2 pos, int targetType) {
    int count = 0;
    for (int dy = -neighborhoodRadius; dy <= neighborhoodRadius; ++dy) { // Изменили циклы
        for (int dx = -neighborhoodRadius; dx <= neighborhoodRadius; ++dx) { // Изменили циклы
            if (dx == 0 && dy == 0) continue;
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
    int neighborsCount[8];
    for (int i = 1; i <= 7; i++) {
        neighborsCount[i] = countLiveNeighbors(pos, i);
    }

    int maxCount = 0;
    int equalTypes[8];
    int equalCount = 0;
    for (int i = 1; i <= 7; i++) {
        if (neighborsCount[i] > maxCount) {
            maxCount = neighborsCount[i];
            equalCount = 0;
            equalTypes[equalCount] = i; // Сохраняем новый макс. тип
            equalCount++;
        } else if (neighborsCount[i] == maxCount && maxCount != 0) {
            equalTypes[equalCount] = i; // Сохраняем равный тип
            equalCount++;
        }
    }
    if(maxCount == 0) return 0; //если нет соседей, то умирает

    if (equalCount > 1) {
        int randomIndex = int(float(equalCount) * fract(sin(dot(pos, vec2(12.9898, 78.233))) * 43758.5453));
        return equalTypes[randomIndex]; // Возвращаем случайный тип из равных
    } else {
        return equalTypes[0]; // Возвращаем единственный максимальный тип
    }
}

// Метод для генерации цвета на основе типа клетки.
vec4 getColorByType(int type, int currentState) {
    if (type == 0) {
        return vec4(0.05, 0.05, 0.08, 0.0); // Мертвая клетка
    }
    if (type < 0) {
        return vec4(0.0, 0.0, 0.0, 0.0); // Пустая клетка
    }


    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
    float increase = 0.02;

    int t = type;
    int r = (t / 4) % 2;
    int g = (t / 2) % 2;
    int b = (t / 1) % 2;

    color.r = float(r);
    color.g = float(g);
    color.b = float(b);

    if(currentState > 0 && currentState == type){
        vec4 currentColor = colors[gl_GlobalInvocationID.y * gridSize.x + gl_GlobalInvocationID.x];
        color.r = min(currentColor.r + increase, color.r);
        color.g = min(currentColor.g + increase, color.g);
        color.b = min(currentColor.b + increase, color.b);
    }
    if (type > 7){
        color = vec4(1.0,1.0,1.0,0.0);
    }
    if(type > 0){
        color = clamp(color,vec4(0,0,0,0),vec4(1,1,1,0));
    }
    return color;
}


void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= gridSize.x || pos.y >= gridSize.y) return;

    int index = pos.y * gridSize.x + pos.x;
    int currentState = current[index];
    int neighbors = countLiveNeighbors(pos, 0); // Все живые соседи
    int nextState = currentState;

    if (useAdvancedRules == 1) {
        // Расширенный режим (B/S)
        if (currentState > 0) {
            if (canSurvive(neighbors) && !isOverpopulated(neighbors)) nextState = currentState;
            else nextState = 0;
        } else if (canBeBorn(neighbors)) {
            nextState = determineNewType(pos);
        }
    } else {
        // обычный режим
        if (currentState > 0) { // Живая клетка
            if (neighbors >= survivalMin && neighbors <= survivalMax && neighbors < overpopulation) {
                nextState = currentState; // Сохраняем тип
            } else {
                nextState = 0; // Умирает
            }
        } else if (currentState <= 0) { // Мёртвая клетка
            if (neighbors == birth) {
                nextState = determineNewType(pos); // Оживает с типом
            }
        }
    }

    next[index] = nextState;
    colors[index] = getColorByType(nextState, currentState);
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
        cells[index] = -1; // Мертвая клетка
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


vec4 getColorByType(int type) {
    if (type == 0) {
        return vec4(0.05, 0.05, 0.08, 0.0); // Мертвая клетка
    }
    if (type < 0) {
        return vec4(0.0, 0.0, 0.0, 0.0); // Пустая клетка
    }

    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    int r = (type / 4) % 2;
    int g = (type / 2) % 2;
    int b = (type / 1) % 2;
    color.r = float(r);
    color.g = float(g);
    color.b = float(b);

    if (type > 9){
        color = vec4(1.0,1.0,1.0,0.0);
    }

    return color;
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
            colors[index] = getColorByType(1);
        } else if (typeFloat < 0.285) { // 2/7
            cells[index] = 2; // Красный
            colors[index] = getColorByType(2);
        } else if (typeFloat < 0.428) { // 3/7
            cells[index] = 3; // Синий
            colors[index] = getColorByType(3);
        } else if (typeFloat < 0.571) { // 4/7
            cells[index] = 4; // Жёлтый
            colors[index] = getColorByType(4);
        } else if (typeFloat < 0.714) { // 5/7
            cells[index] = 5; // Оранжевый
            colors[index] = getColorByType(5);
        } else if (typeFloat < 0.857) { // 6/7
            cells[index] = 6; // Фиолетовый
            colors[index] = getColorByType(7);
        } else { // 7/7
            cells[index] = 7; // Белый
            colors[index] = vec4(0.9, 0.9, 0.9, 1.0);
        }
    } else {
        cells[index] = -1; // пустая клетка
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
    glUniform1i(glGetUniformLocation(computeProgram, "neighborhoodRadius"), neighborhoodRadius);
    // переменные для расширенного режима правил
    glUniform1i(glGetUniformLocation(computeProgram, "useAdvancedRules"), useAdvancedRules);
    glUniform1iv(glGetUniformLocation(computeProgram, "birthCounts"), 9, birthRules);
    glUniform1iv(glGetUniformLocation(computeProgram, "surviveCounts"), 9, surviveRules);
    glUniform1iv(glGetUniformLocation(computeProgram, "overpopulationCounts"), 9, overpopulationRules);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellsBuffer[currentBufferIndex]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellsBuffer[(currentBufferIndex + 1) % 2]);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, colorsBuffer);

    glDispatchCompute((gridWidth + groupSizeX - 1) / groupSizeX,
        (gridHeight + groupSizeY - 1) / groupSizeY, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    SwapGridBuffers();
    //glFlush();
}

void GPUAutomaton::SwapGridBuffers() {
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

//001 - синий(type = 1)
//010 - зелёный(type = 2)
//011 - голубой(type = 3)
//100 - красный(type = 4)
//101 - фиолетовый(type = 5)
//110 - жёлтый(type = 6)
//111 - белый(type = 7)
void GPUAutomaton::SetCellType(int x, int y, int type) {
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
        std::cerr << "SetCellType: Invalid coordinates (" << x << ", " << y << ")" << std::endl;
        return;
    }
    if (type < -1 || type > 7) {
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

    colorData[0] = (float)((type / 4) % 2) / 2.0f; // Делим на 2.0f
    colorData[1] = (float)((type / 2) % 2) / 2.0f; // Делим на 2.0f
    colorData[2] = (float)((type / 1) % 2) / 2.0f; // Делим на 2.0f
    colorData[3] = 0.0f;

    if (type == 0) { // Мёртвая
        colorData[0] = 0.05f; colorData[1] = 0.05f; colorData[2] = 0.08f; colorData[3] = 0.0f;
    }
    else if (type == -1) { // пустая
        colorData[0] = 0.0f; colorData[1] = 0.0f; colorData[2] = 0.0f; colorData[3] = 0.0f;
    }

    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorsBuffer));
    GL_CHECK(glBufferSubData(GL_SHADER_STORAGE_BUFFER, index * sizeof(float) * 4, sizeof(float) * 4, colorData));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}