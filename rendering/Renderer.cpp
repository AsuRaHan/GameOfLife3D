#include "Renderer.h"

Renderer::Renderer(int width, int height)
    : width(width), height(height), farPlane(999009000000.0f), camera(45.0f, static_cast<float>(width) / height, 0.1f, farPlane),
    pGameController(nullptr), uiRenderer(nullptr), selectionRenderer(camera, shaderManager)
{
    SetupOpenGL();
    OnWindowResize(width, height);
}

Renderer::~Renderer() {
    glDeleteProgram(cellShaderProgram);
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
    pGameController = gameController;
    LoadShaders();

    InitializeCellsVBOs();
    InitializeGridVBOs();

    uiRenderer = UIRenderer(pGameController);
    uiRenderer.InitializeUI();

    pGameController = gameController;
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

    // Инициализация VAO для клеток
    GL_CHECK(glGenVertexArrays(1, &cellsVAO));
    GL_CHECK(glBindVertexArray(cellsVAO)); // Сначала привязываем VAO

    // Инициализация VBO для вершин клеток
    GL_CHECK(glGenBuffers(1, &cellsVBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellsVBO));

    //float scale_factor = 0.1f;
    //float centerX = 0.5f;
    //float centerY = 0.5f;
    float vertices[] = {
        0.1f, 0.1f,
        0.9f, 0.1f,
        0.9f, 0.9f,
        0.1f, 0.9f
    };
    //float vertices[] = {
    //    centerX + (0.1f - centerX) * (1 - scale_factor), centerY + (0.1f - centerY) * (1 - scale_factor), // Вершина 1
    //    centerX + (0.5f - centerX) * (1 - scale_factor), centerY + (0.0f - centerY) * (1 - scale_factor), // Вершина 5
    //    centerX + (0.9f - centerX) * (1 - scale_factor), centerY + (0.1f - centerY) * (1 - scale_factor), // Вершина 2
    //    centerX + (1.0f - centerX) * (1 - scale_factor), centerY + (0.5f - centerY) * (1 - scale_factor), // Вершина 6
    //    centerX + (0.9f - centerX) * (1 - scale_factor), centerY + (0.9f - centerY) * (1 - scale_factor), // Вершина 3
    //    centerX + (0.5f - centerX) * (1 - scale_factor), centerY + (1.0f - centerY) * (1 - scale_factor), // Вершина 7
    //    centerX + (0.1f - centerX) * (1 - scale_factor), centerY + (0.9f - centerY) * (1 - scale_factor), // Вершина 4
    //    centerX + (0.0f - centerX) * (1 - scale_factor), centerY + (0.5f - centerY) * (1 - scale_factor)  // Вершина 8
    //};

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    // Настройка атрибутов для вершин квадрата клетки
    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr));
    GL_CHECK(glEnableVertexAttribArray(0));

    // Инициализация VBO для данных инстанса
    GL_CHECK(glGenBuffers(1, &cellInstanceVBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));

    cellInstances.clear();
    cellInstances.reserve(gridWidth * gridHeight);
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            Cell cell = pGameController->getGrid().getCell(x, y);
            // Предполагаем, что getColor() возвращает Vector3d
            cellInstances.push_back({
                x * cellSize, y * cellSize,
                {cell.getColor().X(), cell.getColor().Y(), cell.getColor().Z()}
                });
        }
    }

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cellInstances.size() * sizeof(CellInstance), cellInstances.data(), GL_STATIC_DRAW));

    // Настройка атрибутов для инстансинга
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
    GL_CHECK(glVertexAttribDivisor(1, 1)); // Каждый инстанс имеет свою позицию
    GL_CHECK(glEnableVertexAttribArray(1));

    GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, color))));
    GL_CHECK(glVertexAttribDivisor(3, 1)); // Цвет для каждого инстанса
    GL_CHECK(glEnableVertexAttribArray(3));

    // Отвязываем буферы и VAO
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0));
}

void Renderer::InitializeGridVBOs() {
    if (!pGameController) return;
    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    gridVertices.clear();

    // Определение шагов для дополнительных линий
    int minorStep = 10; // Например, линии через каждые 10 единиц
    int majorStep = 100; // Например, более заметные линии через каждые 50 единиц

    // Создание сетки
    for (int y = 0; y <= gridHeight; ++y) {
        for (int x = 0; x <= gridWidth; ++x) {
            // Добавляем информацию о "величине" линии в каждую вершину 
            float majorLine = (x % majorStep == 0 || y % majorStep == 0) ? 1.0f : 0.0f;
            float minorLine = (x % minorStep == 0 || y % minorStep == 0) ? 1.0f : 0.0f;

            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine); // Добавляем значения для шейдера

            // Добавляем следующую вершину для линии
            gridVertices.push_back((x + 1) * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);

            // Повторяем для вертикальных линий
            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);

            gridVertices.push_back(x * cellSize); gridVertices.push_back((y + 1) * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
        }
    }

    // Создание VAO и VBO для сетки
    GL_CHECK(glGenVertexArrays(1, &gridVAO));
    GL_CHECK(glGenBuffers(1, &gridVBO));
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, gridVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW));

    // Настраиваем атрибуты вершин
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0)); // Position
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)))); // Major line flag
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)))); // Minor line flag
    GL_CHECK(glEnableVertexAttribArray(2));
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
    // Используем шейдерную программу для сетки
    GL_CHECK(glUseProgram(gridShaderProgram));

    // Передаем матрицы проекции и вида в шейдеры
    GLuint projectionLoc = glGetUniformLocation(gridShaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(gridShaderProgram, "view");
    GLuint cameraDistanceLoc = glGetUniformLocation(gridShaderProgram, "cameraDistance"); // uniform для дистанции камеры

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));

    float cameraDistance = camera.GetDistance();
    GL_CHECK(glUniform1f(cameraDistanceLoc, cameraDistance));

    // Связываем VAO сетки
    GL_CHECK(glBindVertexArray(gridVAO));

    // Рисуем линии сетки
    GL_CHECK(glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(gridVertices.size() / 5))); // Теперь каждый элемент состоит из 5 float

    // Отвязываем VAO
    GL_CHECK(glBindVertexArray(0));
}

void Renderer::DrawCells() {
    if (!pGameController) return;
    int GW = pGameController->getGridWidth();
    // Привязываем VAO перед настройкой атрибутов
    GL_CHECK(glBindVertexArray(cellsVAO));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (ptr) {
        memcpy(ptr, cellInstances.data(), cellInstances.size() * sizeof(CellInstance));
        GL_CHECK(glUnmapBuffer(GL_ARRAY_BUFFER));
    }

    GL_CHECK(glUseProgram(cellShaderProgram));

    // Устанавливаем uniform переменные
    GLuint projectionLoc = glGetUniformLocation(cellShaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(cellShaderProgram, "view");
    GLuint cellSizeLoc = glGetUniformLocation(cellShaderProgram, "cellSize");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
    GL_CHECK(glUniform1f(cellSizeLoc, pGameController->getCellSize()));

    // Настраиваем атрибуты для вершин квадрата клетки
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellsVBO));
    GL_CHECK(glEnableVertexAttribArray(0)); // Вершины квадрата
    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr));

    // Настраиваем атрибуты для инстансинга (позиция и цвет)
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    GL_CHECK(glEnableVertexAttribArray(1)); // Позиция инстансов
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
    GL_CHECK(glVertexAttribDivisor(1, 1));

    GL_CHECK(glEnableVertexAttribArray(3)); // Цвет инстансов
    GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, color))));
    GL_CHECK(glVertexAttribDivisor(3, 1));

    // Отрисовка клеток с использованием инстансинга
    //GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 8, cellInstances.size()));
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, static_cast<GLsizei>(cellInstances.size())));
    // Отключаем использование атрибутов после отрисовки
    GL_CHECK(glDisableVertexAttribArray(0));
    GL_CHECK(glDisableVertexAttribArray(1));
    GL_CHECK(glDisableVertexAttribArray(3));
}

void Renderer::OnWindowResize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    GL_CHECK(glViewport(0, 0, width, height));
    // Обновляем проекцию камеры
    camera.SetProjection(45.0f, static_cast<float>(width) / height, 0.1f, farPlane);
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
    cellShaderProgram = shaderManager.getProgram("cellShaderProgram");

}

void Renderer::LoadGridShaders() {
    GLuint gridVertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint gridFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const std::string gridVertexShaderSource = R"(
#version 330 core
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
    drawMedium = (cameraDistance > 250.0) ? 0.0 : 1.0; // Средние линии видны, если камера ближе 150 единиц
}
    )";

    const std::string gridFragmentShaderSource = R"(
#version 330 core
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

    // Освобождаем старые буферы
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteBuffers(1, &gridVBO);

    // Пересоздаем буферы
    InitializeCellsVBOs();
    InitializeGridVBOs();
}