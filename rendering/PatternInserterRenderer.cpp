#include "PatternInserterRenderer.h"

PatternInserterRenderer::PatternInserterRenderer(const Camera& camera, ShaderManager& shaderManager)
    : BaseRenderer(camera, shaderManager),
    VAO(0), VBO(0), outlineVAO(0), outlineVBO(0), patternShaderProgram(0),
    cellType(1), previewPosition(0.0f, 0.0f, 0.0f) {
    LoadShaders();
}

PatternInserterRenderer::~PatternInserterRenderer() {
    DeleteBuffers(VAO, VBO);
    DeleteBuffers(outlineVAO, outlineVBO);
}

void PatternInserterRenderer::Initialize() {
    // Инициализация буфера для клеток
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // Позиция
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Цвет
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Инициализация буфера для контура
    glGenVertexArrays(1, &outlineVAO);
    glGenBuffers(1, &outlineVBO);
    glBindVertexArray(outlineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // Позиция
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Цвет
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PatternInserterRenderer::UpdateBuffers() {
    if (currentPattern.empty() || !pGameController) return;

    int height = currentPattern.size();
    int width = currentPattern[0].size();
    vertices.clear();

    float cellSize = pGameController->getCellSize();
    float r, g, b;
    //if (cellType == 1) { r = 0.0f; g = 0.5f; b = 0.0f; } // Зелёный
    //else if (cellType == 2) { r = 0.5f; g = 0.0f; b = 0.0f; } // Красный
    //else if (cellType == 3) { r = 0.0f; g = 0.0f; b = 0.5f; } // Синий
    //else if (cellType == 4) { r = 0.5f; g = 0.5f; b = 0.0f; } // Жёлтый
    //else if (cellType == 5) { r = 0.5f; g = 0.0f; b = 0.5f; } // Оранжевый
    //else if (cellType == 6) { r = 0.0f; g = 0.5f; b = 0.5f; } // Фиолетовый
    //else { r = 1.0f; g = 1.0f; b = 1.0f; } // Белый (cellType == 7)
    r = (float)((cellType / 4) % 2) / 2.0f; // Делим на 2.0f
    g = (float)((cellType / 2) % 2) / 2.0f; // Делим на 2.0f
    b = (float)((cellType / 1) % 2) / 2.0f; // Делим на 2.0f
    

    float inset = 0.2f; // Отступ 0.2 от края клетки (на каждую сторону)

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (currentPattern[y][x]) {
                float xPos = (float)(width - 1 - x) * cellSize; // Инвертируем x
                float yPos = (float)(height - 1 - y) * cellSize; // Инвертируем y

                // Уменьшаем квадратик с каждой стороны
                float xMin = xPos + inset * cellSize; // Левый край с отступом
                float xMax = xPos + cellSize - inset * cellSize; // Правый край с отступом
                float yMin = yPos + inset * cellSize; // Нижний край с отступом
                float yMax = yPos + cellSize - inset * cellSize; // Верхний край с отступом

                // Первый треугольник
                vertices.push_back(xMin); vertices.push_back(yMin); vertices.push_back(0.0f); // Нижний левый
                vertices.push_back(r);    vertices.push_back(g);    vertices.push_back(b);
                vertices.push_back(xMax); vertices.push_back(yMin); vertices.push_back(0.0f); // Нижний правый
                vertices.push_back(r);    vertices.push_back(g);    vertices.push_back(b);
                vertices.push_back(xMax); vertices.push_back(yMax); vertices.push_back(0.0f); // Верхний правый
                vertices.push_back(r);    vertices.push_back(g);    vertices.push_back(b);

                // Второй треугольник
                vertices.push_back(xMax); vertices.push_back(yMax); vertices.push_back(0.0f); // Верхний правый
                vertices.push_back(r);    vertices.push_back(g);    vertices.push_back(b);
                vertices.push_back(xMin); vertices.push_back(yMax); vertices.push_back(0.0f); // Верхний левый
                vertices.push_back(r);    vertices.push_back(g);    vertices.push_back(b);
                vertices.push_back(xMin); vertices.push_back(yMin); vertices.push_back(0.0f); // Нижний левый
                vertices.push_back(r);    vertices.push_back(g);    vertices.push_back(b);
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Обновляем контур после клеток
    UpdateOutlineBuffers();
}

void PatternInserterRenderer::UpdateOutlineBuffers() {
    if (currentPattern.empty() || !pGameController) return;

    int height = currentPattern.size();
    int width = currentPattern[0].size();
    outlineVertices.clear();

    float cellSize = pGameController->getCellSize();
    float r = 1.0f, g = 1.0f, b = 0.0f; // Жёлтый цвет для контура

    // Координаты прямоугольника (без инверсии, т.к. уже учтено в клетках)
    float xMin = 0.0f; // Левый край
    float xMax = (float)width * cellSize; // Правый край
    float yMin = 0.0f; // Нижний край
    float yMax = (float)height * cellSize; // Верхний край

    // Четыре линии (8 вершин для GL_LINES)
    // Нижняя линия
    outlineVertices.push_back(xMin); outlineVertices.push_back(yMin); outlineVertices.push_back(0.0f);
    outlineVertices.push_back(r);    outlineVertices.push_back(g);    outlineVertices.push_back(b);
    outlineVertices.push_back(xMax); outlineVertices.push_back(yMin); outlineVertices.push_back(0.0f);
    outlineVertices.push_back(r);    outlineVertices.push_back(g);    outlineVertices.push_back(b);

    // Правая линия
    outlineVertices.push_back(xMax); outlineVertices.push_back(yMin); outlineVertices.push_back(0.0f);
    outlineVertices.push_back(r);    outlineVertices.push_back(g);    outlineVertices.push_back(b);
    outlineVertices.push_back(xMax); outlineVertices.push_back(yMax); outlineVertices.push_back(0.0f);
    outlineVertices.push_back(r);    outlineVertices.push_back(g);    outlineVertices.push_back(b);

    // Верхняя линия
    outlineVertices.push_back(xMax); outlineVertices.push_back(yMax); outlineVertices.push_back(0.0f);
    outlineVertices.push_back(r);    outlineVertices.push_back(g);    outlineVertices.push_back(b);
    outlineVertices.push_back(xMin); outlineVertices.push_back(yMax); outlineVertices.push_back(0.0f);
    outlineVertices.push_back(r);    outlineVertices.push_back(g);    outlineVertices.push_back(b);

    // Левая линия
    outlineVertices.push_back(xMin); outlineVertices.push_back(yMax); outlineVertices.push_back(0.0f);
    outlineVertices.push_back(r);    outlineVertices.push_back(g);    outlineVertices.push_back(b);
    outlineVertices.push_back(xMin); outlineVertices.push_back(yMin); outlineVertices.push_back(0.0f);
    outlineVertices.push_back(r);    outlineVertices.push_back(g);    outlineVertices.push_back(b);

    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    glBufferData(GL_ARRAY_BUFFER, outlineVertices.size() * sizeof(float), outlineVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PatternInserterRenderer::Draw() {
    if (!pGameController || !pGameController->IsPatternPlacementMode()) return;

    auto newPattern = pGameController->getCurrentPattern();
    int newCellType = pGameController->getCellType();
    if (newPattern != currentPattern || newCellType != cellType) {
        currentPattern = newPattern;
        cellType = newCellType;
        UpdateBuffers(); // Обновляем и клетки, и контур
    }

    if (currentPattern.empty()) return;

    previewPosition = pGameController->GetPatternPreviewPosition();

    glUseProgram(patternShaderProgram);
    SetCommonUniforms(patternShaderProgram);

    float cellSize = pGameController->getCellSize();
    Vector3d worldPos = previewPosition * cellSize;

    GLint offsetLoc = GetUniformLocation(patternShaderProgram, "offset");
    glUniform2f(offsetLoc, worldPos.X(), worldPos.Y());

    GLint timeLoc = GetUniformLocation(patternShaderProgram, "time");
    float time = (float)GetTickCount64() / 1000.0f;
    glUniform1f(timeLoc, time);

    // Рисуем клетки
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
    glBindVertexArray(0);

    // Рисуем контур
    glBindVertexArray(outlineVAO);
    glDrawArrays(GL_LINES, 0, outlineVertices.size() / 6); // 8 вершин = 4 линии
    glBindVertexArray(0);
}

void PatternInserterRenderer::LoadShaders() {
    const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 projection;
uniform mat4 view;
uniform vec2 offset;
out vec3 fragColor;

void main() {
    vec3 worldPos = aPos + vec3(offset.x, offset.y, 0.0);
    gl_Position = projection * view * vec4(worldPos, 1.0);
    fragColor = aColor;
}
    )";

    const char* fragmentShaderSource = R"(
#version 330 core
in vec3 fragColor;
out vec4 FragColor;
uniform float time;

void main() {
    float pulse = 0.8 + 0.2 * sin(time * 3.0); // Пульсация яркости
    FragColor = vec4(fragColor * pulse, 0.7);
}
    )";

    patternShaderProgram = LoadShaderProgram("patternShaderProgram", vertexShaderSource, fragmentShaderSource);
}