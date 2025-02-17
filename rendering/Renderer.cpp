#include "Renderer.h"

Renderer::Renderer(int width, int height)
    : width(width), height(height), farPlane(999009000000.0f), camera(45.0f, static_cast<float>(width) / height, 0.1f, farPlane),
    pGameController(nullptr), uiRenderer(nullptr), selectionRenderer(camera, shaderManager)
    , debugTextureShaderProgram(0), debugTextureVAO(0), debugTextureVBO(0), debugTextureID(0)
{
    SetupOpenGL();
    OnWindowResize(width, height);
}

Renderer::~Renderer() {
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteVertexArrays(1, &debugOverlayVAO);

    glDeleteVertexArrays(1, &debugTextureVAO);
    glDeleteBuffers(1, &debugTextureVBO);
    glDeleteTextures(1, &debugTextureID);
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

    // Инициализация отладочной текстуры
    LoadDebugTextureShaders();
    InitializeDebugTexture();
}

void Renderer::SetupOpenGL() {
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // Черный фон
    GL_CHECK(glEnable(GL_DEPTH_TEST)); // Включаем тест глубины
    GL_CHECK(glViewport(0, 0, width, height));

    // Проверка максимального количества текстурных единиц
    GLint maxTextureUnits;
    GL_CHECK(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits));
    std::cout << "Max Texture Units: " << maxTextureUnits << std::endl;
}

//void Renderer::InitializeCellsVBOs() {
//    if (!pGameController) return;
//    int gridWidth = pGameController->getGridWidth();
//    int gridHeight = pGameController->getGridHeight();
//    float cellSize = pGameController->getCellSize();
//
//    GL_CHECK(glGenVertexArrays(1, &cellsVAO));
//    GL_CHECK(glBindVertexArray(cellsVAO));
//
//    GL_CHECK(glGenBuffers(1, &cellsVBO));
//    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellsVBO));
//
//    float vertices[] = {
//        0.1f, 0.1f,
//        0.9f, 0.1f,
//        0.9f, 0.9f,
//        0.1f, 0.9f
//    };
//
//    //GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
//    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW));
//
//    // Настраиваем атрибуты вершин один раз при инициализации
//    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr));
//    GL_CHECK(glEnableVertexAttribArray(0));
//
//    GL_CHECK(glGenBuffers(1, &cellInstanceVBO));
//    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
//
//    cellInstances.clear();
//    cellInstances.reserve(gridWidth * gridHeight);
//    for (int y = 0; y < gridHeight; ++y) {
//        for (int x = 0; x < gridWidth; ++x) {
//            Cell cell = pGameController->getGrid().getCell(x, y);
//            cellInstances.push_back({
//                x * cellSize, y * cellSize,
//                {cell.getColor().X(), cell.getColor().Y(), cell.getColor().Z()}
//                });
//        }
//    }
//
//    //GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cellInstances.size() * sizeof(CellInstance), cellInstances.data(), GL_STATIC_DRAW));
//    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cellInstances.size() * sizeof(CellInstance), cellInstances.data(), GL_DYNAMIC_DRAW));
//
//    // Настраиваем атрибуты инстансинга один раз при инициализации
//    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
//    GL_CHECK(glVertexAttribDivisor(1, 1));
//    GL_CHECK(glEnableVertexAttribArray(1));
//
//    GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, color))));
//    GL_CHECK(glVertexAttribDivisor(3, 1));
//    GL_CHECK(glEnableVertexAttribArray(3));
//
//    GL_CHECK(glBindVertexArray(0)); // Отвязываем VAO
//}
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
    cellInstances.reserve(gridWidth * gridHeight);
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            cellInstances.push_back({
                x * cellSize, y * cellSize,
                {0.0f, 0.0f, 0.0f} // Цвета теперь берутся из colorsBuffer, здесь можно оставить 0
                });
        }
    }

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

    gridVertices.clear();

    int minorStep = 10;
    int majorStep = 100;

    for (int y = 0; y <= gridHeight; ++y) {
        for (int x = 0; x <= gridWidth; ++x) {
            float majorLine = (x % majorStep == 0 || y % majorStep == 0) ? 1.0f : 0.0f;
            float minorLine = (x % minorStep == 0 || y % minorStep == 0) ? 1.0f : 0.0f;

            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);

            gridVertices.push_back((x + 1) * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);

            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);

            gridVertices.push_back(x * cellSize); gridVertices.push_back((y + 1) * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
        }
    }

    GL_CHECK(glGenVertexArrays(1, &gridVAO));
    GL_CHECK(glGenBuffers(1, &gridVBO));
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, gridVBO));
    //GL_CHECK(glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW));

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

    // Отрисовка отладочной текстуры
    DrawDebugTexture();

    SwapBuffers(wglGetCurrentDC());
}

void Renderer::DrawGrid() {
    GL_CHECK(glUseProgram(gridShaderProgram));

    GLuint projectionLoc = glGetUniformLocation(gridShaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(gridShaderProgram, "view");
    GLuint cameraDistanceLoc = glGetUniformLocation(gridShaderProgram, "cameraDistance");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));

    float cameraDistance = camera.GetDistance();
    GL_CHECK(glUniform1f(cameraDistanceLoc, cameraDistance));

    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(gridVertices.size() / 5)));
    GL_CHECK(glBindVertexArray(0));
}

//void Renderer::DrawCells() {
//    if (!pGameController) return;
//
//    GL_CHECK(glBindVertexArray(cellsVAO));
//
//    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
//    void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
//    if (ptr) {
//        memcpy(ptr, cellInstances.data(), cellInstances.size() * sizeof(CellInstance));
//        GL_CHECK(glUnmapBuffer(GL_ARRAY_BUFFER));
//    }
//
//    GL_CHECK(glUseProgram(cellShaderProgram));
//
//    GLuint projectionLoc = glGetUniformLocation(cellShaderProgram, "projection");
//    GLuint viewLoc = glGetUniformLocation(cellShaderProgram, "view");
//    GLuint cellSizeLoc = glGetUniformLocation(cellShaderProgram, "cellSize");
//
//    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
//    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
//    GL_CHECK(glUniform1f(cellSizeLoc, pGameController->getCellSize()));
//
//    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, static_cast<GLsizei>(cellInstances.size())));
//
//    GL_CHECK(glBindVertexArray(0)); // Отвязываем VAO
//}
void Renderer::DrawCells() {
    if (!pGameController) return;

    GL_CHECK(glBindVertexArray(cellsVAO));

    // Привязываем буфер позиций инстансов
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribDivisor(1, 1)); // Позиция обновляется для каждого инстанса

    // Привязываем буфер цветов из GPUAutomaton
    GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer));
    GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
    GL_CHECK(glEnableVertexAttribArray(3));
    GL_CHECK(glVertexAttribDivisor(3, 1)); // Цвет обновляется для каждого инстанса

    GL_CHECK(glUseProgram(cellShaderProgram));

    GLuint projectionLoc = glGetUniformLocation(cellShaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(cellShaderProgram, "view");
    GLuint cellSizeLoc = glGetUniformLocation(cellShaderProgram, "cellSize");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
    GL_CHECK(glUniform1f(cellSizeLoc, pGameController->getCellSize()));

    // Убедитесь, что количество экземпляров соответствует размеру сетки
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
    LoadCellShaders();
    LoadGridShaders();
    LoadDebugTextureShaders();
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

    std::cout << "Rebuilding game field..." << std::endl;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // Обновляем VBO для клеток
    GL_CHECK(glBindVertexArray(cellsVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));

    cellInstances.clear();
    cellInstances.reserve(gridWidth * gridHeight);
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            Cell cell = pGameController->getGrid().getCell(x, y);
            cellInstances.push_back({
                x * cellSize, y * cellSize,
                {cell.getColor().X(), cell.getColor().Y(), cell.getColor().Z()}
                });
        }
    }

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cellInstances.size() * sizeof(CellInstance), cellInstances.data(), GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0));

    // Обновляем VBO для сетки
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, gridVBO));

    gridVertices.clear();
    int minorStep = 10;
    int majorStep = 100;

    for (int y = 0; y <= gridHeight; ++y) {
        for (int x = 0; x <= gridWidth; ++x) {
            float majorLine = (x % majorStep == 0 || y % majorStep == 0) ? 1.0f : 0.0f;
            float minorLine = (x % minorStep == 0 || y % minorStep == 0) ? 1.0f : 0.0f;

            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);

            gridVertices.push_back((x + 1) * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);

            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);

            gridVertices.push_back(x * cellSize); gridVertices.push_back((y + 1) * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
        }
    }

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0));

    std::cout << "Game field rebuilt." << std::endl;
}

void Renderer::InitializeDebugTexture() {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();

    // Создание VAO и VBO для прямоугольника
    glGenVertexArrays(1, &debugTextureVAO);
    glGenBuffers(1, &debugTextureVBO);
    glBindVertexArray(debugTextureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugTextureVBO);

    float vertices[] = {
        0.5f, -1.0f, 0.0f, 1.0f, // Нижний правый угол
        1.0f, -1.0f, 1.0f, 1.0f, // Верхний правый угол
        1.0f,  1.0f, 1.0f, 0.0f, // Верхний левый угол
        0.5f,  1.0f, 0.0f, 0.0f  // Нижний левый угол
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Создание текстуры
    glGenTextures(1, &debugTextureID);

    // Установка активной текстурной единицы
    glActiveTexture(GL_TEXTURE1);

    // Привязка текстуры
    glBindTexture(GL_TEXTURE_2D, debugTextureID);

    // Создание данных текстуры в памяти (красный цвет)
    std::vector<float> textureData(gridWidth * gridHeight * 3);
    for (int i = 0; i < gridWidth * gridHeight; ++i) {
        textureData[i * 3] = 1.0f;     // R
        textureData[i * 3 + 1] = 0.0f; // G
        textureData[i * 3 + 2] = 0.0f; // B
    }

    // Настройка текстуры
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gridWidth, gridHeight, 0, GL_RGB, GL_FLOAT, textureData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Отвязка текстуры
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::LoadDebugTextureShaders() {
    const std::string vertexShaderSource = R"(
#version 430 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
    )";

    const std::string fragmentShaderSource = R"(
#version 430 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D textureSampler;
void main()
{
    FragColor = texture(textureSampler, TexCoord);
}
    )";

    shaderManager.loadVertexShader("debugTextureVertexShader", vertexShaderSource.c_str());
    shaderManager.loadFragmentShader("debugTextureFragmentShader", fragmentShaderSource.c_str());
    shaderManager.linkProgram("debugTextureShaderProgram", "debugTextureVertexShader", "debugTextureFragmentShader");
    debugTextureShaderProgram = shaderManager.getProgram("debugTextureShaderProgram");
}

void Renderer::DrawDebugTexture() {
    if (!pGameController) return;

    // Установка активной текстурной единицы
    glActiveTexture(GL_TEXTURE1);

    // Привязка текстуры
    glBindTexture(GL_TEXTURE_2D, debugTextureID);

    // Установка шейдера и отрисовка
    glUseProgram(debugTextureShaderProgram);
    glUniform1i(glGetUniformLocation(debugTextureShaderProgram, "textureSampler"), 1);

    glBindVertexArray(debugTextureVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    // Отвязка текстуры
    glBindTexture(GL_TEXTURE_2D, 0);
}