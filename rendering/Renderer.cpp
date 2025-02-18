#include "Renderer.h"

Renderer::Renderer(int width, int height)
    : width(width), height(height), farPlane(999009000000.0f), camera(45.0f, static_cast<float>(width) / height, 0.1f, farPlane),
    pGameController(nullptr), uiRenderer(nullptr), selectionRenderer(camera, shaderManager)
    , debugTextureShaderProgram(0), debugTextureVAO(0), debugTextureVBO(0), debugTextureID(0), debugTextureOffsetX(0.0f)
{
    SetupOpenGL();
    OnWindowResize(width, height);
}

Renderer::~Renderer() {
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &gridVBO);

    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteVertexArrays(1, &gridVAO);

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

    selectionRenderer.SetGameController(gameController);

    // Инициализация отладочной текстуры
    //LoadDebugTextureShaders();
    //InitializeDebugTexture();
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
    cellInstances.reserve(gridWidth * gridHeight);
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            cellInstances.push_back({ x * cellSize, y * cellSize }); // Удален instanceColor
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

    const int minorStep = 10;
    const int majorStep = 100;

    gridVertices.reserve((gridWidth + 1) * (gridHeight + 1) * 10);

    // Объединенный цикл для рисования горизонтальных и вертикальных линий
    for (int y = 0; y <= gridHeight; ++y) {
        for (int x = 0; x <= gridWidth; ++x) {
            float xPos = x * cellSize;
            float yPos = y * cellSize;

            // Горизонтальные линии
            if (x < gridWidth) {
                float majorLine = (y % majorStep == 0) ? 1.0f : 0.0f;
                float minorLine = (y % minorStep == 0) ? 1.0f : 0.0f;

                gridVertices.push_back(xPos); gridVertices.push_back(yPos); gridVertices.push_back(0.0f);
                gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
                gridVertices.push_back(xPos + cellSize); gridVertices.push_back(yPos); gridVertices.push_back(0.0f);
                gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
            }

            // Вертикальные линии
            if (y < gridHeight) {
                float majorLine = (x % majorStep == 0) ? 1.0f : 0.0f;
                float minorLine = (x % minorStep == 0) ? 1.0f : 0.0f;

                gridVertices.push_back(xPos); gridVertices.push_back(yPos); gridVertices.push_back(0.0f);
                gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
                gridVertices.push_back(xPos); gridVertices.push_back(yPos + cellSize); gridVertices.push_back(0.0f);
                gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
            }
        }
    }

    // Настройка OpenGL буферов (без изменений)
    GL_CHECK(glGenVertexArrays(1, &gridVAO));
    GL_CHECK(glGenBuffers(1, &gridVBO));
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, gridVBO));
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

    // Отрисовка отладочной текстуры

    DrawCells();
    if (pGameController->IsSelectionActive()) {
        selectionRenderer.SetIsSelecting(pGameController->IsSelectionActive());
        selectionRenderer.Draw();
    }
    // Отрисовка сетки
    if (pGameController->getShowGrid())DrawGrid();
    //DrawDebugTexture();
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
    GLuint cameraDistanceLoc = glGetUniformLocation(gridShaderProgram, "cameraDistance");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
    GL_CHECK(glUniform1f(cameraDistanceLoc, camera.GetDistance()));

    // Связываем VAO сетки
    GL_CHECK(glBindVertexArray(gridVAO));
    // Рисуем линии сетки
    GL_CHECK(glDrawArrays(GL_LINES, 0, gridVertices.size() / 5)); // gridVertices содержит координаты вершин для линий сетки
    // Отвязываем VAO
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

    std::cout << "Rebuilding game field..." << std::endl;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // 1. Подготавливаем данные для клеток и сетки
    cellInstances.clear();
    cellInstances.reserve(gridWidth * gridHeight);

    gridVertices.clear();
    int minorStep = 10;
    int majorStep = 100;
    gridVertices.reserve((gridWidth + 1) * (gridHeight + 1) * 10); // Примерный размер для сетки

    for (int y = 0; y <= gridHeight; ++y) {
        for (int x = 0; x <= gridWidth; ++x) {
            // Добавляем позицию клетки (если внутри границ)
            if (x < gridWidth && y < gridHeight) {
                cellInstances.push_back({ x * cellSize, y * cellSize });
            }

            float xPos = x * cellSize;
            float yPos = y * cellSize;
            // Горизонтальные линии
            if (x < gridWidth) {
                float majorLine = (y % majorStep == 0) ? 1.0f : 0.0f;
                float minorLine = (y % minorStep == 0) ? 1.0f : 0.0f;

                gridVertices.push_back(xPos); gridVertices.push_back(yPos); gridVertices.push_back(0.0f);
                gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
                gridVertices.push_back(xPos + cellSize); gridVertices.push_back(yPos); gridVertices.push_back(0.0f);
                gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
            }

            // Вертикальные линии
            if (y < gridHeight) {
                float majorLine = (x % majorStep == 0) ? 1.0f : 0.0f;
                float minorLine = (x % minorStep == 0) ? 1.0f : 0.0f;

                gridVertices.push_back(xPos); gridVertices.push_back(yPos); gridVertices.push_back(0.0f);
                gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
                gridVertices.push_back(xPos); gridVertices.push_back(yPos + cellSize); gridVertices.push_back(0.0f);
                gridVertices.push_back(majorLine); gridVertices.push_back(minorLine);
            }
        }
    }

    // 2. Обновляем VBO для клеток
    GL_CHECK(glBindVertexArray(cellsVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cellInstances.size() * sizeof(CellInstance), cellInstances.data(), GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0));

    // 3. Обновляем VBO для сетки
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, gridVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0));

    std::cout << "Game field rebuilt. new size Width=" << gridWidth << " Height=" << gridHeight << std::endl;
}


void Renderer::InitializeDebugTexture() {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    debugTextureOffsetX = gridWidth * cellSize;

    glGenVertexArrays(1, &debugTextureVAO);
    glGenBuffers(1, &debugTextureVBO);
    glBindVertexArray(debugTextureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugTextureVBO);

    float vertices[] = {
        0.0f, 0.0f,             0.0f, 1.0f,
        gridWidth * cellSize, 0.0f,             1.0f, 1.0f,
        0.0f, gridHeight * cellSize, 0.0f, 0.0f,
        gridWidth * cellSize, gridHeight * cellSize, 1.0f, 0.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTextures(1, &debugTextureID);
    glBindTexture(GL_TEXTURE_2D, debugTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gridWidth, gridHeight, 0, GL_RGBA, GL_FLOAT, nullptr); // Изменено на GL_RGBA
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}


void Renderer::LoadDebugTextureShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        uniform mat4 projection;
        uniform mat4 view;
        uniform float offsetX; // Смещение по X
        out vec2 TexCoord;
        void main() {
            vec2 pos = aPos + vec2(offsetX, 0.0); // Смещаем позицию текстуры
            gl_Position = projection * view * vec4(pos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D textureSampler;
        void main() {
            FragColor = texture(textureSampler, TexCoord);
        }
    )";

    shaderManager.loadVertexShader("debugTextureVertexShader", vertexShaderSource);
    shaderManager.loadFragmentShader("debugTextureFragmentShader", fragmentShaderSource);
    shaderManager.linkProgram("debugTextureShaderProgram", "debugTextureVertexShader", "debugTextureFragmentShader");
    debugTextureShaderProgram = shaderManager.getProgram("debugTextureShaderProgram");
}


void Renderer::DrawDebugTexture() {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();

    GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
    std::vector<float> textureData(gridWidth * gridHeight * 4); // Изменено на 4 компонента
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorsBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, textureData.size() * sizeof(float), textureData.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, debugTextureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gridWidth, gridHeight, GL_RGBA, GL_FLOAT, textureData.data());

    glUseProgram(debugTextureShaderProgram);

    GLint projectionLoc = glGetUniformLocation(debugTextureShaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(debugTextureShaderProgram, "view");
    GLint offsetLoc = glGetUniformLocation(debugTextureShaderProgram, "offsetX");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix());
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix());
    glUniform1f(offsetLoc, debugTextureOffsetX);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, debugTextureID);

    GLint textureLoc = glGetUniformLocation(debugTextureShaderProgram, "textureSampler");
    glUniform1i(textureLoc, 0);

    glBindVertexArray(debugTextureVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}