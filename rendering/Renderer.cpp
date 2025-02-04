#include "Renderer.h"

Renderer::Renderer(int width, int height)
    : width(width), height(height), camera(45.0f, static_cast<float>(width) / height, 0.1f, 999009000000.0f),
    pGameController(nullptr){
    SetupOpenGL();
    initializeUI();
    OnWindowResize(width, height);
}

Renderer::~Renderer() {
    glDeleteProgram(shaderProgram);
    glDeleteProgram(gridShaderProgram);
    glDeleteProgram(debugOverlayShaderProgram);
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteVertexArrays(1, &debugOverlayVAO);
}

void Renderer::SetCamera(const Camera& camera) {
    this->camera = camera;
}

void Renderer::SetGameController(GameController* gameController) {
    this->pGameController = gameController;

    LoadShaders();
    InitializeVBOs();
    
}

void Renderer::SetupOpenGL() {
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // Черный фон
    GL_CHECK(glEnable(GL_DEPTH_TEST)); // Включаем тест глубины
    GL_CHECK(glViewport(0, 0, width, height));
}

void Renderer::InitializeVBOs() {
    if (!pGameController) return;
    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // Инициализация VBO для клеток
    GL_CHECK(glGenBuffers(1, &cellsVBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellsVBO));
    //float vertices[] = {
    //    0.1f, 0.1f,
    //    0.9f, 0.1f,
    //    0.9f, 0.9f,
    //    0.1f, 0.9f
    //};
    float scale_factor = 0.2f;
    float centerX = 0.5f;
    float centerY = 0.5f;

    float vertices[] = {
        centerX + (0.1f - centerX) * (1 - scale_factor), centerY + (0.1f - centerY) * (1 - scale_factor), // Вершина 1
        centerX + (0.5f - centerX) * (1 - scale_factor), centerY + (0.0f - centerY) * (1 - scale_factor), // Вершина 5
        centerX + (0.9f - centerX) * (1 - scale_factor), centerY + (0.1f - centerY) * (1 - scale_factor), // Вершина 2
        centerX + (1.0f - centerX) * (1 - scale_factor), centerY + (0.5f - centerY) * (1 - scale_factor), // Вершина 6
        centerX + (0.9f - centerX) * (1 - scale_factor), centerY + (0.9f - centerY) * (1 - scale_factor), // Вершина 3
        centerX + (0.5f - centerX) * (1 - scale_factor), centerY + (1.0f - centerY) * (1 - scale_factor), // Вершина 7
        centerX + (0.1f - centerX) * (1 - scale_factor), centerY + (0.9f - centerY) * (1 - scale_factor), // Вершина 4
        centerX + (0.0f - centerX) * (1 - scale_factor), centerY + (0.5f - centerY) * (1 - scale_factor)  // Вершина 8
    };

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    // Инициализация VBO для данных инстанса
    GL_CHECK(glGenBuffers(1, &cellInstanceVBO));
    cellInstances.clear();
    cellInstances.reserve(gridWidth * gridHeight);
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            Cell cell = pGameController->getGrid().getCell(x, y);
            cellInstances.push_back({
                x * cellSize, y * cellSize,
                cell.getColor()
                });
        }
    }

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cellInstances.size() * sizeof(CellInstance), cellInstances.data(), GL_STATIC_DRAW));

    InitializeGridVBOs();
    InitializeDebugOverlay();
}

void Renderer::InitializeGridVBOs() {
    if (!pGameController) return;
    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    gridVertices.clear();
    std::vector<float> majorGridVertices;

    // Создание сетки
    for (int y = 0; y <= gridHeight; ++y) {
        for (int x = 0; x <= gridWidth; ++x) {
            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back((x + 1) * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(x * cellSize); gridVertices.push_back((y + 1) * cellSize); gridVertices.push_back(0.0f);
        }
    }

    // Создание "главных" линий
    for (int y = 0; y <= gridHeight; y += 10) {
        for (int x = 0; x <= gridWidth; x += 10) {
            majorGridVertices.push_back(x * cellSize); majorGridVertices.push_back(y * cellSize); majorGridVertices.push_back(0.0f);
            majorGridVertices.push_back((x == gridWidth ? x : x + 10) * cellSize); majorGridVertices.push_back(y * cellSize); majorGridVertices.push_back(0.0f);
            majorGridVertices.push_back(x * cellSize); majorGridVertices.push_back(y * cellSize); majorGridVertices.push_back(0.0f);
            majorGridVertices.push_back(x * cellSize); majorGridVertices.push_back((y == gridHeight ? y : y + 10) * cellSize); majorGridVertices.push_back(0.0f);
        }
    }

    // Создание VAO и VBO для обычной сетки
    GL_CHECK(glGenVertexArrays(1, &gridVAO));
    GL_CHECK(glGenBuffers(1, &gridVBO));
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, gridVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));

    // Создание VAO и VBO для "главных" линий
    GLuint majorGridVAO, majorGridVBO;
    GL_CHECK(glGenVertexArrays(1, &majorGridVAO));
    GL_CHECK(glGenBuffers(1, &majorGridVBO));
    GL_CHECK(glBindVertexArray(majorGridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, majorGridVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, majorGridVertices.size() * sizeof(float), majorGridVertices.data(), GL_STATIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));

    // Теперь вам нужно будет отрисовывать оба VAO в методе DrawGrid:
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glDrawArrays(GL_LINES, 0, gridVertices.size() / 3));
    GL_CHECK(glBindVertexArray(majorGridVAO));
    GL_CHECK(glDrawArrays(GL_LINES, 0, majorGridVertices.size() / 3));
    GL_CHECK(glBindVertexArray(0));
}

void Renderer::Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Отрисовка сетки и клеток
    if(showGrid)DrawGrid();

    DrawCells();

    DrawDebugOverlay();

    // Отрисовка GUI поверх всего остального
    if (uiManager) {
        glDisable(GL_DEPTH_TEST);  // GUI всегда поверх
        uiManager->draw();
        glEnable(GL_DEPTH_TEST);
    }

    SwapBuffers(wglGetCurrentDC());
}

void Renderer::DrawGrid() {
    // Используем шейдерную программу для сетки
    GL_CHECK(glUseProgram(gridShaderProgram));

    // Передаем матрицы проекции и вида в шейдеры
    GLuint projectionLoc = glGetUniformLocation(gridShaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(gridShaderProgram, "view");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));

    // Связываем VAO сетки
    GL_CHECK(glBindVertexArray(gridVAO));

    // Рисуем линии сетки
    GL_CHECK(glDrawArrays(GL_LINES, 0, gridVertices.size() / 3)); // Предполагаем, что gridVertices содержит координаты вершин для линий сетки

    // Отвязываем VAO
    GL_CHECK(glBindVertexArray(0));
}

void Renderer::DrawCells() {
    if (!pGameController) return;

    // Обновляем данные о клетках
    for (size_t i = 0; i < cellInstances.size(); ++i) {
        int x = i % pGameController->getGridWidth();
        int y = i / pGameController->getGridWidth();
        Cell cell = pGameController->getGrid().getCell(x, y);
        cellInstances[i].color = cell.getColor(); // Обновляем только цвет
    }

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (ptr) {
        memcpy(ptr, cellInstances.data(), cellInstances.size() * sizeof(CellInstance));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    GL_CHECK(glUseProgram(shaderProgram));

    // Устанавливаем uniform переменные
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint cellSizeLoc = glGetUniformLocation(shaderProgram, "cellSize");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
    GL_CHECK(glUniform1f(cellSizeLoc, pGameController->getCellSize()));

    // Настраиваем атрибуты для вершин квадрата клетки
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellsVBO));
    GL_CHECK(glEnableVertexAttribArray(0)); // Вершины квадрата (позиция)
    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr));

    // Настраиваем атрибуты для инстансинга
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    GL_CHECK(glEnableVertexAttribArray(1)); // Позиция инстанса
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
    GL_CHECK(glVertexAttribDivisor(1, 1)); // Каждый инстанс имеет свою позицию

    GL_CHECK(glEnableVertexAttribArray(3)); // Цвет клетки
    GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, color))));
    GL_CHECK(glVertexAttribDivisor(3, 1)); // Цвет для каждого инстанса

    // Отрисовка клеток с использованием инстансинга
    //GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, cellInstances.size()));
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 8, cellInstances.size()));

    // Отключаем использование атрибутов после отрисовки
    GL_CHECK(glDisableVertexAttribArray(0));
    GL_CHECK(glDisableVertexAttribArray(1));
    GL_CHECK(glDisableVertexAttribArray(3));
}

void Renderer::DrawDebugOverlay() {
    if (!pGameController || !pGameController->isSimulationRunning()) return; // Добавляем проверку
    GL_CHECK(glDisable(GL_DEPTH_TEST)); // Отключаем тест глубины для оверлея

    GL_CHECK(glUseProgram(debugOverlayShaderProgram));

    // Устанавливаем цвет треугольника
    // Например, красный цвет
    GLfloat redColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    GLuint overlayColorLocation = glGetUniformLocation(debugOverlayShaderProgram, "overlayColor");
    GL_CHECK(glUniform4fv(overlayColorLocation, 1, redColor));

    GL_CHECK(glBindVertexArray(debugOverlayVAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3)); // Рисуем треугольник
    GL_CHECK(glBindVertexArray(0));

    GL_CHECK(glEnable(GL_DEPTH_TEST)); // Включаем обратно тест глубины после отрисовки оверлея
}

void Renderer::OnWindowResize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    GL_CHECK(glViewport(0, 0, width, height));
    // Обновляем проекцию камеры
    camera.SetProjection(45.0f, static_cast<float>(width) / height, 0.1f, 100000000.0f);
    UpdateDebugOverlayPosition();
    if (uiManager) {
        uiManager->updateLayout(newWidth, newHeight);
    }
}

void Renderer::MoveCamera(float dx, float dy, float dz) {
    // Обновляем позицию камеры
    float currentX, currentY, currentZ;
    camera.GetPosition(currentX, currentY, currentZ);
    camera.SetPosition(currentX + dx, currentY + dy, currentZ + dz);
}

void Renderer::LoadShaders() {
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
layout(location = 3) in vec3 aInstanceColor; 
uniform mat4 projection;
uniform mat4 view;
uniform float cellSize; 
out vec3 vInstanceColor;
void main()
{
    gl_Position = projection * view * vec4(aPos * cellSize + aInstancePos, 0.0, 1.0);
    vInstanceColor = aInstanceColor;
}
    )";

    const std::string fragmentShaderSource = R"(
#version 330 core
in vec3 vInstanceColor;
out vec4 FragColor;
void main()
{
    FragColor = vec4(vInstanceColor, 1.0);
}
    )";
    shaderManager.loadVertexShader("cellVertexShader", vertexShaderSource.c_str());
    shaderManager.loadFragmentShader("cellFragmentShader", fragmentShaderSource.c_str());
    shaderManager.linkProgram("cellShaderProgram", "cellVertexShader", "cellFragmentShader");
    shaderProgram = shaderManager.getProgram("cellShaderProgram");

}

void Renderer::LoadGridShaders() {
    GLuint gridVertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint gridFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const std::string gridVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 projection;
uniform mat4 view;
out float isMajorLine; // Выходной параметр для фрагментного шейдера
void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    // Пример: рассчитываем, является ли линия "главной"
    isMajorLine = mod(aPos.x, 5.0) == 0.0 || mod(aPos.y, 5.0) == 0.0 ? 1.0 : 0.0;
}
    )";

    const std::string gridFragmentShaderSource = R"(
#version 330 core
in float isMajorLine;
out vec4 FragColor;
void main()
{
    if(isMajorLine > 0.25) {
        FragColor = vec4(0.6, 0.6, 0.7, 1.0); // Красный цвет для "главных" линий
    } else {
        FragColor = vec4(0.3, 0.3, 0.4, 1.0); // Серый цвет для обычных линий
    }
}
    )";
    shaderManager.loadVertexShader("gridVertexShader", gridVertexShaderSource.c_str());
    shaderManager.loadFragmentShader("gridFragmentShader", gridFragmentShaderSource.c_str());
    shaderManager.linkProgram("gridShaderProgram", "gridVertexShader", "gridFragmentShader");
    gridShaderProgram = shaderManager.getProgram("gridShaderProgram");

}

void Renderer::InitializeDebugOverlay() {
    //std::string vertexShaderSource = LoadShaderSource("./glsl/overlay_vertex_shader.glsl");
    const std::string vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
}
        )";
    //std::string fragmentShaderSource = LoadShaderSource("./glsl/overlay_fragment_shader.glsl");
    const std::string fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 overlayColor;
void main()
{
    FragColor = overlayColor;
}
        )";
    shaderManager.loadVertexShader("debugOverlayVertexShader", vertexShaderSource.c_str());
    shaderManager.loadFragmentShader("debugOverlayFragmentShader", fragmentShaderSource.c_str());
    shaderManager.linkProgram("debugOverlayShaderProgram", "debugOverlayVertexShader", "debugOverlayFragmentShader");
    debugOverlayShaderProgram = shaderManager.getProgram("debugOverlayShaderProgram");

    // Инициализация VAO и VBO для маленького треугольника в правом нижнем углу
    float xOffset = 0.95f; // Смещение по X для правого нижнего угла
    float yOffset = -0.95f; // Смещение по Y для правого нижнего угла
    float size = 0.05f; // Размер треугольника в NDC

    // Координаты вершин для треугольника, направленного острой стороной вправо
    float vertices[] = {
        xOffset - size, yOffset,       // Левая вершина
        xOffset, yOffset + size,       // Верхняя вершина
        xOffset, yOffset               // Правая вершина
    };

    glGenVertexArrays(1, &debugOverlayVAO);
    glGenBuffers(1, &debugOverlayVBO);

    glBindVertexArray(debugOverlayVAO);

    glBindBuffer(GL_ARRAY_BUFFER, debugOverlayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Позиция
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    UpdateDebugOverlayPosition();
}

void Renderer::UpdateDebugOverlayPosition() {
    // Удаляем старый VBO, если он существует
    if (debugOverlayVBO != 0) {
        glDeleteBuffers(1, &debugOverlayVBO);
    }

    // Размер треугольника в пикселях
    const int triangleSizeInPixels = 20; // Например, 20 пикселей

    // Вычисляем смещение и размер в нормализованных координатах
    float xOffset = (2.0f * (width - triangleSizeInPixels) / width) - 1.0f; // Правый край - размер треугольника
    float yOffset = (2.0f * (triangleSizeInPixels) / height) - 1.0f; // Нижний край + размер треугольника
    float sizeX = (2.0f * triangleSizeInPixels / width); // Размер по X в нормализованных координатах
    float sizeY = (2.0f * triangleSizeInPixels / height); // Размер по Y в нормализованных координатах

    float vertices[] = {
        xOffset, yOffset + sizeY, // Верхняя вершина
        xOffset - sizeX, yOffset, // Левая нижняя вершина
        xOffset, yOffset          // Правая нижняя вершина
    };

    glBindVertexArray(debugOverlayVAO);

    glGenBuffers(1, &debugOverlayVBO);
    glBindBuffer(GL_ARRAY_BUFFER, debugOverlayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Позиция
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Renderer::RebuildGameField() {
    if (!pGameController) return;

    // Освобождаем старые буферы
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteBuffers(1, &gridVBO);

    InitializeVBOs(); // Перестраиваем буферы
}

void Renderer::initializeUI() {
    uiManager = std::make_shared<UIManager>();

    // Создаем кнопки управления
    auto startButton = std::make_unique<Button>(-0.8f, 0.8f, 0.2f, 0.1f, "Start", Vector3d(0.2f, 0.7f, 0.2f));
    startButton->setOnClick([this]() { 
        if (pGameController->isSimulationRunning()) {
            pGameController->stopSimulation();
        }
        else {
            pGameController->startSimulation();
        }
    });

    auto clearButton = std::make_unique<Button>(-0.5f, 0.8f, 0.2f, 0.1f, "Clear", Vector3d(0.7f, 0.2f, 0.2f));
    clearButton->setOnClick([this]() { 
        if (pGameController) pGameController->clearGrid();
    });

    auto stepButton = std::make_unique<Button>(-0.2f, 0.8f, 0.2f, 0.1f, "Step", Vector3d(0.2f, 0.2f, 0.7f));
    stepButton->setOnClick([this]() { 
        if (pGameController) pGameController->stepSimulation();
    });

    // Создаем метки для отображения информации
    auto generationLabel = std::make_unique<Label>(0.7f, 0.8f, "Generation: 0", Vector3d(1.0f, 1.0f, 1.0f));
    auto speedLabel = std::make_unique<Label>(0.7f, 0.7f, "Speed: 1x", Vector3d(1.0f, 1.0f, 1.0f));

    // Добавляем элементы в UI менеджер
    uiManager->addElement(std::move(startButton));
    uiManager->addElement(std::move(clearButton));
    uiManager->addElement(std::move(stepButton));
    uiManager->addElement(std::move(generationLabel));
    uiManager->addElement(std::move(speedLabel));
}

// Обработка кликов мыши
void Renderer::OnMouseClick(int x, int y) {
    if (uiManager) {
        float normalizedX, normalizedY;
        UIElement::screenToNormalized(x, y, width, height, normalizedX, normalizedY);
        uiManager->handleClick(normalizedX, normalizedY);
    }
}

void Renderer::OnMouseMove(int x, int y) {
    if (uiManager) {
        float normalizedX, normalizedY;
        UIElement::screenToNormalized(x, y, width, height, normalizedX, normalizedY);
        uiManager->handleMouseMove(normalizedX, normalizedY);
    }
}