#include "GridRenderer.h"

GridRenderer::GridRenderer(const Camera& camera, ShaderManager& shaderManager)
    : BaseRenderer(camera, shaderManager), // Передаем параметры в конструктор базового класса
    gridVAO(0), gridVBO(0), gridShaderProgram(0) {
    LoadGridShaders();
}

GridRenderer::~GridRenderer() {
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);
}

void GridRenderer::Initialize() {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // Вычисляем количество вершин для сетки
    gridVertexCount = ((gridWidth + 1) * 2 + (gridHeight + 1) * 2) * 5;
    gridBufferSize = gridVertexCount * sizeof(float);

    // Настройка OpenGL буферов
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);

    gridVertices.clear();
    gridVertices.resize(gridVertexCount);

    CreateOrUpdateGridVerticesUsingComputeShader(gridVertices);

    glBufferData(GL_ARRAY_BUFFER, gridBufferSize, gridVertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GridRenderer::Draw() {
    if (!pGameController || !pGameController->getShowGrid()) return;

    glUseProgram(gridShaderProgram);
    SetCommonUniforms(gridShaderProgram);

    GLint cameraDistanceLoc = GetUniformLocation(gridShaderProgram, "cameraDistance");
    glUniform1f(cameraDistanceLoc, camera.GetDistance());

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridVertexCount / 5);
    glBindVertexArray(0);
}

void GridRenderer::LoadGridShaders() {
    // ===================================================================================================================

    const char* vertexShaderSource = R"(
#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in float isMajorLine;
layout(location = 2) in float isMinorLine;
uniform mat4 projection;
uniform mat4 view;
uniform float cameraDistance;
out float majorLine;
out float minorLine;
out float drawMinor;
out float drawMedium;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    majorLine = isMajorLine;
    minorLine = isMinorLine;
    drawMinor = (cameraDistance > 70.0) ? 0.0 : 1.0;
    drawMedium = (cameraDistance > 140.0) ? 0.0 : 1.0;
}
    )";

    // ===================================================================================================================
    const char* fragmentShaderSource = R"(
#version 430 core
in float majorLine;
in float minorLine;
in float drawMinor;
in float drawMedium;
out vec4 FragColor;

void main()
{
    if(majorLine > 0.5) {
        FragColor = vec4(0.8, 0.8, 0.8, 0.7);
    } else if(minorLine > 0.5 && drawMedium > 0.5) {
        FragColor = vec4(0.5, 0.5, 0.5, 1.0);
    } else if(drawMinor > 0.5) {
        FragColor = vec4(0.2, 0.2, 0.2, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
    )";

    gridShaderProgram = LoadShaderProgram("gridShaderProgram", vertexShaderSource, fragmentShaderSource);
    // ===================================================================================================================

    shaderManager.loadComputeShader("computeGridShaderProgram", R"(

#version 430 core

layout(local_size_x = 32, local_size_y = 32) in; 

layout(std430, binding = 0) buffer GridVertices {
    float vertices[];
};

uniform int gridWidth;
uniform int gridHeight;
uniform float cellSize;
uniform int majorStep;
uniform int minorStep;

void main() {
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;

    // Проверяем, что x и y находятся в пределах сетки
    if (x > gridWidth || y > gridHeight) return;
    
    uint baseIndexForVertical = x * 10; // Начало для вертикальных линий
    uint baseIndexForHorizontal = (gridWidth + 1) * 10 + y * 10; // Начало для горизонтальных линий после вертикальных, включая дополнительные линии

    // Добавляем вертикальные линии
    if (x <= gridWidth) { // Мы используем <=, чтобы включить дополнительную линию справа
        float xPos = x * cellSize;
        float majorLine = float(x % majorStep == 0);
        float minorLine = float(x % minorStep == 0) * (1.0 - majorLine); // minorLine true только если не majorLine

        // Вертикальные линии
        vertices[baseIndexForVertical + 0] = xPos;
        vertices[baseIndexForVertical + 1] = 0.0;
        vertices[baseIndexForVertical + 2] = 0.0;
        vertices[baseIndexForVertical + 3] = majorLine;
        vertices[baseIndexForVertical + 4] = minorLine;

        vertices[baseIndexForVertical + 5] = xPos;
        vertices[baseIndexForVertical + 6] = gridHeight * cellSize;
        vertices[baseIndexForVertical + 7] = 0.0;
        vertices[baseIndexForVertical + 8] = majorLine;
        vertices[baseIndexForVertical + 9] = minorLine;
    }

    // Добавляем горизонтальные линии
    if (y <= gridHeight) { // Мы используем <=, чтобы включить дополнительную линию сверху
        float yPos = y * cellSize;
        float majorLine = float(y % majorStep == 0);
        float minorLine = float(y % minorStep == 0) * (1.0 - majorLine); // minorLine true только если не majorLine

        // Горизонтальные линии
        vertices[baseIndexForHorizontal + 0] = 0.0;
        vertices[baseIndexForHorizontal + 1] = yPos;
        vertices[baseIndexForHorizontal + 2] = 0.0;
        vertices[baseIndexForHorizontal + 3] = majorLine;
        vertices[baseIndexForHorizontal + 4] = minorLine;

        vertices[baseIndexForHorizontal + 5] = (gridWidth) * cellSize;
        vertices[baseIndexForHorizontal + 6] = yPos;
        vertices[baseIndexForHorizontal + 7] = 0.0;
        vertices[baseIndexForHorizontal + 8] = majorLine;
        vertices[baseIndexForHorizontal + 9] = minorLine;
    }
}

)");

    shaderManager.linkComputeProgram("computeGridProgram", "computeGridShaderProgram");
    computeGridShaderProgram = shaderManager.getProgram("computeGridProgram");
}

void GridRenderer::CreateOrUpdateGridVerticesUsingComputeShader(std::vector<float>& gridVertices) {
    if (!pGameController) return;

    const int majorStep = 100;
    const int minorStep = 10;

    GLuint gridBuffer;
    glGenBuffers(1, &gridBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gridBufferSize, nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gridBuffer);

    //GLuint computeGridShaderProgram = shaderManager.getProgram("computeGridProgram");
    //if (!computeGridShaderProgram) {
    //    std::cerr << "Compute shader for grid not found!" << std::endl;
    //    return;
    //}

    glUseProgram(computeGridShaderProgram);

    glUniform1i(glGetUniformLocation(computeGridShaderProgram, "gridWidth"), pGameController->getGridWidth());
    glUniform1i(glGetUniformLocation(computeGridShaderProgram, "gridHeight"), pGameController->getGridHeight());
    glUniform1f(glGetUniformLocation(computeGridShaderProgram, "cellSize"), pGameController->getCellSize());
    glUniform1i(glGetUniformLocation(computeGridShaderProgram, "majorStep"), majorStep);
    glUniform1i(glGetUniformLocation(computeGridShaderProgram, "minorStep"), minorStep);

    glDispatchCompute((pGameController->getGridWidth() + 31) / 32, (pGameController->getGridHeight() + 31) / 32, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, gridBufferSize, gridVertices.data());
    glDeleteBuffers(1, &gridBuffer);
}