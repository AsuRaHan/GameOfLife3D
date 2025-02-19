#include "Renderer.h"

Renderer::Renderer(int width, int height)
    : width(width), height(height), farPlane(999009000000.0f), camera(45.0f, static_cast<float>(width) / height, 0.1f, farPlane),
    pGameController(nullptr), uiRenderer(nullptr), selectionRenderer(camera, shaderManager)
{
    SetupOpenGL();
    OnWindowResize(width, height);
}

Renderer::~Renderer() {
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &gridVBO);

    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteVertexArrays(1, &gridVAO);
}

void Renderer::SetCamera(const Camera& camera) {
    this->camera = camera;    
}

void Renderer::SetGameController(GameController* gameController) {
    pGameController = gameController;
    LoadShaders();

    InitializeCellsVBOs();
    InitializeGridVBOs();

    uiRenderer = UIRenderer(pGameController);
    uiRenderer.InitializeUI();

    selectionRenderer.SetGameController(gameController);

}

void Renderer::SetupOpenGL() {
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // Черный фон
    GL_CHECK(glEnable(GL_DEPTH_TEST)); // Включаем тест глубины
    GL_CHECK(glViewport(0, 0, width, height));
}

void Renderer::InitializeCellsVBOs() {
    if (!pGameController) return;
    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    GL_CHECK(glGenVertexArrays(1, &cellsVAO));
    GL_CHECK(glBindVertexArray(cellsVAO));

    GL_CHECK(glGenBuffers(1, &cellsVBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellsVBO));

    float vertices[] = {
        0.1f, 0.1f,
        0.9f, 0.1f,
        0.9f, 0.9f,
        0.1f, 0.9f
    };

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr));
    GL_CHECK(glEnableVertexAttribArray(0));

    GL_CHECK(glGenBuffers(1, &cellInstanceVBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));

    cellInstances.clear();
    //cellInstances.reserve(gridWidth * gridHeight);

    CreateOrUpdateCellInstancesUsingComputeShader(cellInstances);

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cellInstances.size() * sizeof(CellInstance), cellInstances.data(), GL_STATIC_DRAW));

    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
    GL_CHECK(glVertexAttribDivisor(1, 1));
    GL_CHECK(glEnableVertexAttribArray(1));

    GL_CHECK(glBindVertexArray(0));
}

void Renderer::InitializeGridVBOs() {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // Вычисляем количество вершин для сетки
    gridVertexCount = ((gridWidth + 1) * 2 + (gridHeight + 1) * 2) * 5;  // каждый элемент сетки имеет 5 float значений
    gridBufferSize = gridVertexCount * sizeof(float);  // размер буфера в байтах

    // Настройка OpenGL буферов
    GL_CHECK(glGenVertexArrays(1, &gridVAO));
    GL_CHECK(glGenBuffers(1, &gridVBO));
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, gridVBO));

    gridVertices.clear();
    gridVertices.resize(gridVertexCount);  // Устанавливаем правильный размер

    CreateOrUpdateGridVerticesUsingComputeShader(gridVertices);

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, gridBufferSize, gridVertices.data(), GL_DYNAMIC_DRAW));

    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float))));
    GL_CHECK(glEnableVertexAttribArray(2));

    GL_CHECK(glBindVertexArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Renderer::Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawCells();

    if (pGameController->IsSelectionActive()) {
        selectionRenderer.SetIsSelecting(pGameController->IsSelectionActive());
        selectionRenderer.Draw();
    }
    // Отрисовка сетки
    if (pGameController->getShowGrid())DrawGrid();
    // используем UIRenderer для отрисовки UI
    if (pGameController->getShowUI())uiRenderer.DrawUI();

    SwapBuffers(wglGetCurrentDC());
}

void Renderer::DrawGrid() {
    GL_CHECK(glUseProgram(gridShaderProgram));

    GLuint projectionLoc = glGetUniformLocation(gridShaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(gridShaderProgram, "view");
    GLuint cameraDistanceLoc = glGetUniformLocation(gridShaderProgram, "cameraDistance");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
    GL_CHECK(glUniform1f(cameraDistanceLoc, camera.GetDistance()));

    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glDrawArrays(GL_LINES, 0, gridVertexCount / 5));
    GL_CHECK(glBindVertexArray(0));
}

void Renderer::DrawCells() {
    if (!pGameController) return;

    GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
    if (colorsBuffer == 0) {
        std::cerr << "Error: Color buffer not initialized." << std::endl;
        return; // Или выбросьте исключение, если это критическая ошибка
    }

    GL_CHECK(glBindVertexArray(cellsVAO));

    // Привязываем буфер позиций инстансов
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribDivisor(1, 1));

    // Привязываем буфер цветов из GPUAutomaton

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer));
    GL_CHECK(glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr)); // Исправлено: 3 -> 4 (vec3 -> vec4)
    GL_CHECK(glEnableVertexAttribArray(3));
    GL_CHECK(glVertexAttribDivisor(3, 1));

    GL_CHECK(glUseProgram(cellShaderProgram));

    GLuint projectionLoc = glGetUniformLocation(cellShaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(cellShaderProgram, "view");
    GLuint cellSizeLoc = glGetUniformLocation(cellShaderProgram, "cellSize");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
    GL_CHECK(glUniform1f(cellSizeLoc, pGameController->getCellSize()));

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, gridWidth * gridHeight));

    GL_CHECK(glBindVertexArray(0));
}

void Renderer::OnWindowResize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    GL_CHECK(glViewport(0, 0, width, height));
    // Обновляем проекцию камеры
    camera.SetProjection(45.0f, static_cast<float>(width) / height, 0.1f, farPlane);
}

void Renderer::LoadShaders() {
    LoadComputeShader();
    LoadCellShaders();
    LoadGridShaders();
}

void Renderer::LoadCellShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const std::string vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos; 
    layout(location = 1) in vec2 aInstancePos;
    layout(location = 3) in vec4 aInstanceColor; // Исправлено: vec3 -> vec4
    uniform mat4 projection;
    uniform mat4 view;
    uniform float cellSize; 
    out vec4 vInstanceColor; // Исправлено: vec3 -> vec4
    void main()
    {
        gl_Position = projection * view * vec4(aPos * cellSize + aInstancePos, 0.0, 1.0);
        vInstanceColor = aInstanceColor;
    }
    )";

    const std::string fragmentShaderSource = R"(
    #version 330 core
    in vec4 vInstanceColor; // Исправлено: vec3 -> vec4
    out vec4 FragColor;
    void main()
    {
        FragColor = vInstanceColor; // Уже vec4, альфа-канал используется
    }
    )";

    shaderManager.loadVertexShader("cellVertexShader", vertexShaderSource.c_str());
    shaderManager.loadFragmentShader("cellFragmentShader", fragmentShaderSource.c_str());
    shaderManager.linkProgram("cellShaderProgram", "cellVertexShader", "cellFragmentShader");
    cellShaderProgram = shaderManager.getProgram("cellShaderProgram");
}

void Renderer::LoadGridShaders() {
    const std::string gridVertexShaderSource = R"(
#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in float isMajorLine; // Флаг для главных линий (самый большой шаг)
layout(location = 2) in float isMinorLine; // Флаг для средних линий (средний шаг)
uniform mat4 projection;
uniform mat4 view;
uniform float cameraDistance; // Расстояние камеры для определения видимости линий
out float majorLine; // Выходной параметр для главных линий
out float minorLine; // Выходной параметр для средних линий
out float drawMinor; // Определяет видимость самых мелких линий
out float drawMedium; // Новая переменная для средних линий

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    majorLine = isMajorLine; // Передаем флаг главных линий во фрагментный шейдер
    minorLine = isMinorLine; // Передаем флаг средних линий во фрагментный шейдер
    
    // Устанавливаем видимость для самых мелких линий
    drawMinor = (cameraDistance > 100.0) ? 0.0 : 1.0; // Самые мелкие линии видны, если камера ближе 100 единиц
    
    // Устанавливаем видимость для средних линий
    drawMedium = (cameraDistance > 250.0) ? 0.0 : 1.0; // Средние линии видны, если камера ближе 250 единиц
}
    )";

    const std::string gridFragmentShaderSource = R"(
#version 430 core
in float majorLine; // Входной параметр для определения главных линий
in float minorLine; // Входной параметр для определения средних линий
in float drawMinor; // Определяет отображение самых мелких линий
in float drawMedium; // Определяет отображение средних линий
out vec4 FragColor;

void main()
{
    if(majorLine > 0.5) {
        FragColor = vec4(0.8, 0.8, 0.8, 1.0); // Цвет для главных линий, всегда видимые
    } else if(minorLine > 0.5 && drawMedium > 0.5) {
        FragColor = vec4(0.5, 0.5, 0.5, 1.0); // Цвет для средних линий, видны если drawMedium > 0.5
    } else if(drawMinor > 0.5) {
        FragColor = vec4(0.2, 0.2, 0.2, 1.0); // Цвет для самых мелких линий, видны если drawMinor > 0.5
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0); // Прозрачный цвет, если линия не видна
    }
}
    )";
    shaderManager.loadVertexShader("gridVertexShader", gridVertexShaderSource.c_str());
    shaderManager.loadFragmentShader("gridFragmentShader", gridFragmentShaderSource.c_str());
    shaderManager.linkProgram("gridShaderProgram", "gridVertexShader", "gridFragmentShader");
    gridShaderProgram = shaderManager.getProgram("gridShaderProgram");
}

void Renderer::RebuildGameField() {
    if (!pGameController) return;

    // Удаляем старые буферы
    GL_CHECK(glDeleteBuffers(1, &cellsVBO));
    GL_CHECK(glDeleteBuffers(1, &cellInstanceVBO));

    GL_CHECK(glDeleteVertexArrays(1, &gridVAO));
    GL_CHECK(glDeleteBuffers(1, &gridVBO));

    // Инициализируем новые буферы
    InitializeCellsVBOs();
    InitializeGridVBOs();
}

void Renderer::LoadComputeShader() {
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
// ==========================================================================
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


void Renderer::CreateOrUpdateCellInstancesUsingComputeShader(std::vector<CellInstance>& cellInstances) {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // Создаем буфер для инстанцирования клеток
    GLuint cellInstanceBuffer;
    glGenBuffers(1, &cellInstanceBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellInstanceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CellInstance) * gridWidth * gridHeight, nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellInstanceBuffer); // Привязка для compute шейдера

    // Используем compute шейдер для заполнения буфера
    glUseProgram(computeCellShaderProgram);

    // Передаем параметры в шейдер
    glUniform1i(glGetUniformLocation(computeCellShaderProgram, "gridWidth"), gridWidth);
    glUniform1i(glGetUniformLocation(computeCellShaderProgram, "gridHeight"), gridHeight);
    glUniform1f(glGetUniformLocation(computeCellShaderProgram, "cellSize"), cellSize);

    // Запускаем вычисления на GPU
    glDispatchCompute((gridWidth + 31) / 32, (gridHeight + 31) / 32, 1); // 32 - размер work group
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Читаем данные обратно на CPU
    cellInstances.resize(gridWidth * gridHeight);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, cellInstances.size() * sizeof(CellInstance), cellInstances.data());

    glDeleteBuffers(1, &cellInstanceBuffer); // Удаляем временный буфер
}

void Renderer::CreateOrUpdateGridVerticesUsingComputeShader(std::vector<float>& gridVertices) {
    if (!pGameController) return;

    // Используем уже сохраненные значения
    const int majorStep = 100;
    const int minorStep = 10;

    GLuint gridBuffer;
    glGenBuffers(1, &gridBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gridBufferSize, nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gridBuffer);

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