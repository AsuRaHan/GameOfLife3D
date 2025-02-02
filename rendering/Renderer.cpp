#include "Renderer.h"
#include <algorithm> // Для std::min

Renderer::Renderer(int width, int height)
    : width(width), height(height), camera(45.0f, static_cast<float>(width) / height, 0.1f, 100000.0f),
    pGameController(nullptr){
    SetupOpenGL();
    
}

Renderer::~Renderer() {
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteBuffers(1, &cellInstanceVBO);
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Черный фон
    glEnable(GL_DEPTH_TEST); // Включаем тест глубины
    glViewport(0, 0, width, height);
}

void Renderer::InitializeVBOs() {
    if (!pGameController) return;
    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // Инициализация VBO для клеток
    glGenBuffers(1, &cellsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);
    float vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Инициализация VBO для данных инстанса
    glGenBuffers(1, &cellInstanceVBO);
    cellInstances.clear();
    cellInstances.reserve(gridWidth * gridHeight);
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            Cell cell = pGameController->getGrid().getCell(x, y);
            cellInstances.push_back({
                x * cellSize, y * cellSize,
                pGameController->getCellState(x, y) ? 1.0f : 0.0f,
                cell.getColor(),
                cell.getType()
                });
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, cellInstances.size() * sizeof(CellInstance), cellInstances.data(), GL_STATIC_DRAW);

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

    // Создание обычной сетки
    for (int y = 0; y <= gridHeight; ++y) {
        for (int x = 0; x <= gridWidth; ++x) {
            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back((x + 1) * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
        }
    }
    for (int y = 0; y <= gridHeight; ++y) {
        for (int x = 0; x <= gridWidth; ++x) {
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
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Создание VAO и VBO для "главных" линий
    GLuint majorGridVAO, majorGridVBO;
    glGenVertexArrays(1, &majorGridVAO);
    glGenBuffers(1, &majorGridVBO);
    glBindVertexArray(majorGridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, majorGridVBO);
    glBufferData(GL_ARRAY_BUFFER, majorGridVertices.size() * sizeof(float), majorGridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Теперь вам нужно будет отрисовывать оба VAO в методе DrawGrid:
    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);
    glBindVertexArray(majorGridVAO);
    glDrawArrays(GL_LINES, 0, majorGridVertices.size() / 3);
    glBindVertexArray(0);
}

void Renderer::Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Отрисовка сетки и клеток
    if(showGrid)DrawGrid();

    DrawCells();

    DrawDebugOverlay();

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

    bool needsUpdate = false;
    // Проверяем, изменилось ли состояние клеток с последнего обновления
    for (size_t i = 0; i < cellInstances.size(); ++i) {
        int x = i % pGameController->getGridWidth();
        int y = i / pGameController->getGridWidth();
        // Получаем текущее состояние клетки
        float newState = pGameController->getCellState(x, y) ? 1.0f : 0.0f;
        if (newState != cellInstances[i].state) {
            cellInstances[i].state = newState;
            // Обновляем цвет клетки
            Cell cell = pGameController->getGrid().getCell(x, y);
            cellInstances[i].color = cell.getColor();
            // Обновляем тип клетки
            cellInstances[i].type = cell.getType();
            needsUpdate = true;
        }
    }

    // Если состояние клеток изменилось, обновляем буфер инстансов
    if (needsUpdate) {
        glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, cellInstances.size() * sizeof(CellInstance), cellInstances.data());
    }

    // Используем шейдерную программу для отрисовки клеток
    GL_CHECK(glUseProgram(shaderProgram));

    // Устанавливаем uniform переменные для матрицы проекции, вида и размера клетки
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint cellSizeLoc = glGetUniformLocation(shaderProgram, "cellSize");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
    GL_CHECK(glUniform1f(cellSizeLoc, pGameController->getCellSize()));

    // Настраиваем атрибуты для вершин квадрата клетки
    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);
    GL_CHECK(glEnableVertexAttribArray(0)); // Вершины квадрата (позиция)
    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr));

    // Настраиваем атрибуты для инстансинга
    glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);
    GL_CHECK(glEnableVertexAttribArray(1)); // Позиция инстанса
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
    GL_CHECK(glVertexAttribDivisor(1, 1)); // Каждый инстанс имеет свою позицию

    GL_CHECK(glEnableVertexAttribArray(2)); // Состояние клетки
    GL_CHECK(glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, state))));
    GL_CHECK(glVertexAttribDivisor(2, 1)); // Состояние для каждого инстанса

    GL_CHECK(glEnableVertexAttribArray(3)); // Цвет клетки
    GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, color))));
    GL_CHECK(glVertexAttribDivisor(3, 1)); // Цвет для каждого инстанса

    GL_CHECK(glEnableVertexAttribArray(4)); // Тип клетки
    GL_CHECK(glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, type))));
    GL_CHECK(glVertexAttribDivisor(4, 1)); // Тип для каждого инстанса

    // Отрисовка клеток с использованием инстансинга
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, cellInstances.size()));

    // Отключаем использование атрибутов после отрисовки
    GL_CHECK(glDisableVertexAttribArray(0));
    GL_CHECK(glDisableVertexAttribArray(1));
    GL_CHECK(glDisableVertexAttribArray(2));
    GL_CHECK(glDisableVertexAttribArray(3));
    GL_CHECK(glDisableVertexAttribArray(4));
}

void Renderer::DrawDebugOverlay() {
    if (!pGameController || !pGameController->isSimulationRunning()) return; // Добавляем проверку
    glDisable(GL_DEPTH_TEST); // Отключаем тест глубины для оверлея

    GL_CHECK(glUseProgram(debugOverlayShaderProgram));

    // Устанавливаем цвет треугольника
    // Например, красный цвет
    GLfloat redColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    GLuint overlayColorLocation = glGetUniformLocation(debugOverlayShaderProgram, "overlayColor");
    GL_CHECK(glUniform4fv(overlayColorLocation, 1, redColor));

    GL_CHECK(glBindVertexArray(debugOverlayVAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3)); // Рисуем треугольник
    GL_CHECK(glBindVertexArray(0));

    glEnable(GL_DEPTH_TEST); // Включаем обратно тест глубины после отрисовки оверлея
}

void Renderer::OnWindowResize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    glViewport(0, 0, width, height);
    // Обновляем проекцию камеры
    camera.SetProjection(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f);
    UpdateDebugOverlayPosition();
}

void Renderer::MoveCamera(float dx, float dy, float dz) {
    // Обновляем позицию камеры
    float currentX, currentY, currentZ;
    camera.GetPosition(currentX, currentY, currentZ);
    camera.SetPosition(currentX + dx, currentY + dy, currentZ + dz);
}

void Renderer::LoadShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //std::string vertexShaderSource = LoadShaderSource("./glsl/cells_vertex_shader.glsl");
    const std::string vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos; 
layout(location = 1) in vec2 aInstancePos;
layout(location = 2) in float aInstanceState; 
layout(location = 3) in vec3 aInstanceColor; 
uniform mat4 projection;
uniform mat4 view;
uniform float cellSize; 
flat out float vInstanceState;
out vec3 vInstanceColor;
void main()
{
    gl_Position = projection * view * vec4(aPos * cellSize + aInstancePos, 0.0, 1.0);
    vInstanceState = aInstanceState;
    vInstanceColor = aInstanceColor;
}
        )";
    //std::string fragmentShaderSource = LoadShaderSource("./glsl/cells_fragment_shader.glsl");
    const std::string fragmentShaderSource = R"(
#version 330 core
flat in float vInstanceState;
in vec3 vInstanceColor;
out vec4 FragColor;
void main()
{
    vec3 color = vInstanceState > 0.5 ? vInstanceColor : vec3(0.1, 0.1, 0.1); // Использование переданного цвета для живых клеток
    FragColor = vec4(color, 1.0);
}
        )";

    const char* vertexSourcePtr = vertexShaderSource.c_str();
    const char* fragmentSourcePtr = fragmentShaderSource.c_str();

    GL_CHECK(glShaderSource(vertexShader, 1, &vertexSourcePtr, NULL));
    GL_CHECK(glCompileShader(vertexShader));
    CheckShaderCompilation(vertexShader, "Vertex Shader");

    GL_CHECK(glShaderSource(fragmentShader, 1, &fragmentSourcePtr, NULL));
    GL_CHECK(glCompileShader(fragmentShader));
    CheckShaderCompilation(fragmentShader, "Fragment Shader");

    shaderProgram = glCreateProgram();
    GL_CHECK(glAttachShader(shaderProgram, vertexShader));
    GL_CHECK(glAttachShader(shaderProgram, fragmentShader));
    GL_CHECK(glLinkProgram(shaderProgram));
    CheckProgramLinking(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
//-----------------------------------------------------------------------------------------------------------------------------//
    GLuint gridVertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint gridFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //std::string gridVertexShaderSource = LoadShaderSource("./glsl/grid_vertex_shader.glsl");
//    const std::string gridVertexShaderSource = R"(
//#version 330 core
//layout(location = 0) in vec3 aPos;
//uniform mat4 projection;
//uniform mat4 view;
//void main()
//{
//    gl_Position = projection * view * vec4(aPos, 1.0);
//}
//        )";
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
    //std::string gridFragmentShaderSource = LoadShaderSource("./glsl/grid_fragment_shader.glsl");
//    const std::string gridFragmentShaderSource = R"(
//#version 330 core
//out vec4 FragColor;
//void main()
//{
//    FragColor = vec4(0.3, 0.3, 0.4, 1.0); // Серый цвет для сетки
//}
//        )";
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
    const char* gridVertexSourcePtr = gridVertexShaderSource.c_str();
    const char* gridFragmentSourcePtr = gridFragmentShaderSource.c_str();

    GL_CHECK(glShaderSource(gridVertexShader, 1, &gridVertexSourcePtr, NULL));
    GL_CHECK(glCompileShader(gridVertexShader));
    CheckShaderCompilation(gridVertexShader, "Grid Vertex Shader");

    GL_CHECK(glShaderSource(gridFragmentShader, 1, &gridFragmentSourcePtr, NULL));
    GL_CHECK(glCompileShader(gridFragmentShader));
    CheckShaderCompilation(gridFragmentShader, "Grid Fragment Shader");

    gridShaderProgram = glCreateProgram();
    GL_CHECK(glAttachShader(gridShaderProgram, gridVertexShader));
    GL_CHECK(glAttachShader(gridShaderProgram, gridFragmentShader));
    GL_CHECK(glLinkProgram(gridShaderProgram));
    CheckProgramLinking(gridShaderProgram);

    glDeleteShader(gridVertexShader);
    glDeleteShader(gridFragmentShader);
}

std::string Renderer::LoadShaderSource(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        //throw std::runtime_error("Failed to open shader file: " + filename);
        std::cout << "Не удалось загрузить файл шейдерной программы: " << filename << std::endl;
        exit(1);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void Renderer::CheckShaderCompilation(GLuint shader, const std::string& name) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::" << name << "::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void Renderer::CheckProgramLinking(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}

void Renderer::InitializeDebugOverlay() {
    // Загрузка и компиляция шейдеров
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

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
    const char* vertexSourcePtr = vertexShaderSource.c_str();
    const char* fragmentSourcePtr = fragmentShaderSource.c_str();

    GL_CHECK(glShaderSource(vertexShader, 1, &vertexSourcePtr, NULL));
    GL_CHECK(glCompileShader(vertexShader));
    CheckShaderCompilation(vertexShader, "Debug Overlay Vertex Shader");

    GL_CHECK(glShaderSource(fragmentShader, 1, &fragmentSourcePtr, NULL));
    GL_CHECK(glCompileShader(fragmentShader));
    CheckShaderCompilation(fragmentShader, "Debug Overlay Fragment Shader");

    debugOverlayShaderProgram = glCreateProgram();
    GL_CHECK(glAttachShader(debugOverlayShaderProgram, vertexShader));
    GL_CHECK(glAttachShader(debugOverlayShaderProgram, fragmentShader));
    GL_CHECK(glLinkProgram(debugOverlayShaderProgram));
    CheckProgramLinking(debugOverlayShaderProgram);

    // После создания и линковки шейдерной программы
    GLuint overlayColorLocation = glGetUniformLocation(debugOverlayShaderProgram, "overlayColor");
    if (overlayColorLocation == -1) {
        std::cout << "Не удалось найти uniform переменную 'overlayColor' в шейдере" << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

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