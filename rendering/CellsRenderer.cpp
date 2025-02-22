#include "CellsRenderer.h"


CellsRenderer::CellsRenderer(const Camera& camera, ShaderManager& shaderManager)
    : BaseRenderer(camera, shaderManager), // Передаем параметры в базовый класс
    cellsVAO(0), cellsVBO(0), cellInstanceVBO(0), cellShaderProgram(0), computeCellShaderProgram(0) {
    LoadCellShaders();
}

CellsRenderer::~CellsRenderer() {
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteVertexArrays(1, &cellsVAO);
}

void CellsRenderer::Initialize() {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    glGenVertexArrays(1, &cellsVAO);
    glBindVertexArray(cellsVAO);

    glGenBuffers(1, &cellsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);

    float vertices[] = {
        0.1f, 0.1f,
        0.9f, 0.1f,
        0.9f, 0.9f,
        0.1f, 0.9f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &cellInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);

    cellInstances.clear();
    CreateOrUpdateCellInstancesUsingComputeShader(cellInstances);

    glBufferData(GL_ARRAY_BUFFER, cellInstances.size() * sizeof(CellInstance), cellInstances.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void CellsRenderer::Draw() {
    if (!pGameController) return;

    GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
    if (colorsBuffer == 0) return;

    glBindVertexArray(cellsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glUseProgram(cellShaderProgram);
    SetCommonUniforms(cellShaderProgram);

    GLint cellSizeLoc = GetUniformLocation(cellShaderProgram, "cellSize");
    glUniform1f(cellSizeLoc, pGameController->getCellSize());

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, gridWidth * gridHeight);

    glBindVertexArray(0);
}

void CellsRenderer::LoadCellShaders() {
    // ===================================================================================================================
    const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos; 
    layout(location = 1) in vec2 aInstancePos;
    layout(location = 3) in vec4 aInstanceColor;
    uniform mat4 projection;
    uniform mat4 view;
    uniform float cellSize; 
    out vec4 vInstanceColor;
    void main()
    {
        gl_Position = projection * view * vec4(aPos * cellSize + aInstancePos, 0.0, 1.0);
        vInstanceColor = aInstanceColor;
    }
    )";

    // ===================================================================================================================
    const char* fragmentShaderSource = R"(
    #version 330 core
    in vec4 vInstanceColor;
    out vec4 FragColor;
    void main()
    {
        FragColor = vInstanceColor;
    }
    )";

    cellShaderProgram = LoadShaderProgram("gridShaderProgram", vertexShaderSource, fragmentShaderSource);
    // ===================================================================================================================
    
    // Compute shader
    shaderManager.loadComputeShader("computeCellShaderProgram", R"(
    #version 430 core
    layout(local_size_x = 32, local_size_y = 32) in;
    layout(std430, binding = 0) buffer Cells {
        vec2 positions[];
    };
    uniform int gridWidth;
    uniform int gridHeight;
    uniform float cellSize;
    void main() {
        uint x = gl_GlobalInvocationID.x;
        uint y = gl_GlobalInvocationID.y;
        if (x >= gridWidth || y >= gridHeight) return;
        uint idx = y * gridWidth + x;
        positions[idx] = vec2(x * cellSize, y * cellSize);
    }
    )");

    shaderManager.linkComputeProgram("computeCellProgram", "computeCellShaderProgram");
    computeCellShaderProgram = shaderManager.getProgram("computeCellProgram");
}

void CellsRenderer::CreateOrUpdateCellInstancesUsingComputeShader(std::vector<CellInstance>& cellInstances) {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    GLuint cellInstanceBuffer;
    glGenBuffers(1, &cellInstanceBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellInstanceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CellInstance) * gridWidth * gridHeight, nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellInstanceBuffer);

    glUseProgram(computeCellShaderProgram);

    glUniform1i(glGetUniformLocation(computeCellShaderProgram, "gridWidth"), gridWidth);
    glUniform1i(glGetUniformLocation(computeCellShaderProgram, "gridHeight"), gridHeight);
    glUniform1f(glGetUniformLocation(computeCellShaderProgram, "cellSize"), cellSize);

    glDispatchCompute((gridWidth + 31) / 32, (gridHeight + 31) / 32, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    cellInstances.resize(gridWidth * gridHeight);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, cellInstances.size() * sizeof(CellInstance), cellInstances.data());

    glDeleteBuffers(1, &cellInstanceBuffer);
}