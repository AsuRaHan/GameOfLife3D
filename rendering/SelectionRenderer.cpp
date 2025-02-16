#include "SelectionRenderer.h"

SelectionRenderer::SelectionRenderer(const Camera& camera, ShaderManager& shaderManager)
    : camera(camera), shaderManager(shaderManager), isSelecting(false)
{
    SetupLineData();
    LoadShaders();
}

SelectionRenderer::~SelectionRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteProgram(selectionShaderProgram);
}

void SelectionRenderer::SetGameController(GameController* gameController) {
    pGameController = gameController;
}

void SelectionRenderer::SetSelectionStart(const Vector3d& start) {
    selectionStart = start;
}

void SelectionRenderer::SetSelectionEnd(const Vector3d& end) {
    selectionEnd = end;
}

void SelectionRenderer::SetIsSelecting(bool isSelecting) {
    this->isSelecting = isSelecting;
}

void SelectionRenderer::Draw() {
    //if (!isSelecting) return; // Не рисуем, если выделение не активно
    if (!isSelecting || pGameController == nullptr) return;

    glUseProgram(selectionShaderProgram);

    // Получаем координаты клеток из GameController
    Vector3d selectionStartCell = pGameController->GetSelectionStart();
    Vector3d selectionEndCell = pGameController->GetSelectionEnd();

    // Преобразуем координаты клеток в мировые координаты
    float cellSize = pGameController->getCellSize();
    Vector3d selectionStartWorld = selectionStartCell * cellSize;
    Vector3d selectionEndWorld = selectionEndCell * cellSize;

    // Устанавливаем uniform переменные для шейдера
    GLint projectionLoc = glGetUniformLocation(selectionShaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(selectionShaderProgram, "view");
    GLint startLoc = glGetUniformLocation(selectionShaderProgram, "selectionStart");
    GLint endLoc = glGetUniformLocation(selectionShaderProgram, "selectionEnd");
    GLint cellSizeLoc = glGetUniformLocation(selectionShaderProgram, "cellSize");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix());
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix());
    glUniform2f(startLoc, selectionStartWorld.X(), selectionStartWorld.Y());
    glUniform2f(endLoc, selectionEndWorld.X(), selectionEndWorld.Y());
    glUniform1f(cellSizeLoc, cellSize); // Передаем размер клетки в шейдер

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 8); // 4 линии = 8 вершин
    glBindVertexArray(0);
}

void SelectionRenderer::SetupLineData() {
    static float vertices[] = {
        0.0f, 0.0f, 0.0f, // Начало первой линии
        1.0f, 0.0f, 0.0f, // Конец первой линии
        1.0f, 0.0f, 0.0f, // Начало второй линии
        1.0f, 1.0f, 0.0f, // Конец второй линии
        1.0f, 1.0f, 0.0f, // Начало третьей линии
        0.0f, 1.0f, 0.0f, // Конец третьей линии
        0.0f, 1.0f, 0.0f, // Начало четвертой линии
        0.0f, 0.0f, 0.0f  // Конец четвертой линии
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SelectionRenderer::LoadShaders() {
    shaderManager.loadVertexShader("selectionVertexShader", R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 projection;
uniform mat4 view;
uniform vec2 selectionStart;
uniform vec2 selectionEnd;
uniform float cellSize;
out vec3 FragPos;

void main()
{
    vec2 start = min(selectionStart, selectionEnd);
    vec2 end = max(selectionStart, selectionEnd);
    vec2 size = (end - start) + vec2(cellSize, cellSize); // Размер выделения с учетом размеров клетки
    
    vec2 pos2d = start + vec2(aPos.x, aPos.y) * size;
    FragPos = vec3(pos2d.x, pos2d.y, 0.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
    )");

    shaderManager.loadFragmentShader("selectionFragmentShader", R"(
    #version 330 core
    out vec4 FragColor;
    void main()
    {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Красный цвет для селектора
    }
    )");

    shaderManager.linkProgram("selectionShaderProgram", "selectionVertexShader", "selectionFragmentShader");
    selectionShaderProgram = shaderManager.getProgram("selectionShaderProgram");
}