#include "CubeRenderer.h"

CubeRenderer::CubeRenderer(const Camera& camera, ShaderManager& shaderManager)
    : BaseRenderer(camera, shaderManager), VAO(0), VBO(0), EBO(0), instanceVBO(0), cubeShaderProgram(0) {
    LoadCubeShaders();
}

CubeRenderer::~CubeRenderer() {
    DeleteBuffers(VAO, VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (instanceVBO) glDeleteBuffers(1, &instanceVBO);
}

void CubeRenderer::Initialize() {
    if (!pGameController) return;

    // Вершины куба, совпадающие с клетками по X и Y (0.1 до 0.9), с подъёмом по Z
    float vertices[] = {
        0.1f, 0.1f, 0.0f,  // Нижний левый ближний
        0.9f, 0.1f, 0.0f,  // Нижний правый ближний
        0.9f, 0.9f, 0.0f,  // Верхний правый ближний
        0.1f, 0.9f, 0.0f,  // Верхний левый ближний
        0.3f, 0.3f, 0.8f,  // Нижний левый дальний
        0.7f, 0.3f, 0.8f,  // Нижний правый дальний
        0.7f, 0.7f, 0.8f,  // Верхний правый дальний
        0.3f, 0.7f, 0.8f   // Верхний левый дальний
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0, // front
        1, 5, 6, 6, 2, 1, // right
        7, 6, 5, 5, 4, 7, // back
        4, 0, 3, 3, 7, 4, // left
        4, 5, 1, 1, 0, 4, // bottom
        3, 2, 6, 6, 7, 3  // top
    };

    std::vector<std::pair<GLint, GLint>> attribs = { {3, 3} };
    CreateBuffers(VAO, VBO, vertices, sizeof(vertices), attribs);

    glBindVertexArray(VAO);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    std::vector<Vector3d> instancePositions(gridWidth * gridHeight);
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            int idx = y * gridWidth + x;
            // Позиция экземпляра — левый нижний угол клетки
            instancePositions[idx] = Vector3d(x * cellSize, y * cellSize, 0.0f); // Z = 0, как у клеток
        }
    }
    glBufferData(GL_ARRAY_BUFFER, instancePositions.size() * sizeof(Vector3d), instancePositions.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3d), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}

void CubeRenderer::LoadCubeShaders() {
    const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aInstancePosition;
    layout(location = 2) in vec4 aInstanceColor;
    uniform mat4 projection;
    uniform mat4 view;
    uniform float scale;
    uniform vec3 cameraPosition;
    out vec3 vColor;
    out vec3 Normal;
    out vec3 FragPos;
    out float vAlpha; // Передаём альфу во фрагментный шейдер
    void main() {
        vec3 worldPosition = aInstancePosition;
        vec3 scaledPosition = aPos * scale + worldPosition;
        FragPos = scaledPosition;
        Normal = normalize(aPos - vec3(0.4, 0.4, 0.4));
        gl_Position = projection * view * vec4(scaledPosition, 1.0);
        vColor = aInstanceColor.rgb;
        vAlpha = aInstanceColor.a; // Передаём альфу
    }
    )";

    const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 vColor;
    in vec3 Normal;
    in vec3 FragPos;
    in float vAlpha; // Получаем альфу
    uniform vec3 cameraPosition;
    uniform vec3 lightColor;
    out vec4 FragColor;
    void main() {
        if (vAlpha < 0.1) discard; // Отбрасываем фрагменты, если клетка мёртва (альфа < 0.1)
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(cameraPosition - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        vec3 result = (0.3 + diffuse) * vColor;
        FragColor = vec4(result, 1.0);
    }
    )";

    cubeShaderProgram = LoadShaderProgram("cubeShaderProgram", vertexShaderSource, fragmentShaderSource);
}

void CubeRenderer::Draw() {
    if (!pGameController) return;

    GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
    if (colorsBuffer == 0) return;

    glBindVertexArray(VAO);

    // Привязываем буфер позиций экземпляров
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3d), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    // Привязываем буфер цветов из GPUAutomaton
    glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glUseProgram(cubeShaderProgram);
    SetCommonUniforms(cubeShaderProgram);

    GLint scaleLoc = GetUniformLocation(cubeShaderProgram, "scale");
    GLint cameraPositionLoc = GetUniformLocation(cubeShaderProgram, "cameraPosition");
    GLint lightColorLoc = GetUniformLocation(cubeShaderProgram, "lightColor");

    float cellSize = pGameController->getCellSize();
    glUniform1f(scaleLoc, cellSize);
    float posX, posY, posZ;
    camera.GetPosition(posX, posY, posZ);
    glUniform3f(cameraPositionLoc, posX, posY, posZ);
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, gridWidth * gridHeight);

    glBindVertexArray(0);
}