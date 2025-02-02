#include "Renderer.h"
#include <algorithm> // ��� std::min

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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // ������ ���
    glEnable(GL_DEPTH_TEST); // �������� ���� �������
    glViewport(0, 0, width, height);
}

void Renderer::InitializeVBOs() {
    if (!pGameController) return;
    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // ������������� VBO ��� ������
    glGenBuffers(1, &cellsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);
    float vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // ������������� VBO ��� ������ ��������
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

    // �������� ������� �����
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

    // �������� "�������" �����
    for (int y = 0; y <= gridHeight; y += 10) {
        for (int x = 0; x <= gridWidth; x += 10) {
            majorGridVertices.push_back(x * cellSize); majorGridVertices.push_back(y * cellSize); majorGridVertices.push_back(0.0f);
            majorGridVertices.push_back((x == gridWidth ? x : x + 10) * cellSize); majorGridVertices.push_back(y * cellSize); majorGridVertices.push_back(0.0f);
            majorGridVertices.push_back(x * cellSize); majorGridVertices.push_back(y * cellSize); majorGridVertices.push_back(0.0f);
            majorGridVertices.push_back(x * cellSize); majorGridVertices.push_back((y == gridHeight ? y : y + 10) * cellSize); majorGridVertices.push_back(0.0f);
        }
    }

    // �������� VAO � VBO ��� ������� �����
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // �������� VAO � VBO ��� "�������" �����
    GLuint majorGridVAO, majorGridVBO;
    glGenVertexArrays(1, &majorGridVAO);
    glGenBuffers(1, &majorGridVBO);
    glBindVertexArray(majorGridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, majorGridVBO);
    glBufferData(GL_ARRAY_BUFFER, majorGridVertices.size() * sizeof(float), majorGridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ������ ��� ����� ����� ������������ ��� VAO � ������ DrawGrid:
    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);
    glBindVertexArray(majorGridVAO);
    glDrawArrays(GL_LINES, 0, majorGridVertices.size() / 3);
    glBindVertexArray(0);
}

void Renderer::Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ��������� ����� � ������
    if(showGrid)DrawGrid();

    DrawCells();

    DrawDebugOverlay();

    SwapBuffers(wglGetCurrentDC());
}

void Renderer::DrawGrid() {
    // ���������� ��������� ��������� ��� �����
    GL_CHECK(glUseProgram(gridShaderProgram));

    // �������� ������� �������� � ���� � �������
    GLuint projectionLoc = glGetUniformLocation(gridShaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(gridShaderProgram, "view");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));

    // ��������� VAO �����
    GL_CHECK(glBindVertexArray(gridVAO));

    // ������ ����� �����
    GL_CHECK(glDrawArrays(GL_LINES, 0, gridVertices.size() / 3)); // ������������, ��� gridVertices �������� ���������� ������ ��� ����� �����

    // ���������� VAO
    GL_CHECK(glBindVertexArray(0));
}


void Renderer::DrawCells() {
    if (!pGameController) return;

    bool needsUpdate = false;
    // ���������, ���������� �� ��������� ������ � ���������� ����������
    for (size_t i = 0; i < cellInstances.size(); ++i) {
        int x = i % pGameController->getGridWidth();
        int y = i / pGameController->getGridWidth();
        // �������� ������� ��������� ������
        float newState = pGameController->getCellState(x, y) ? 1.0f : 0.0f;
        if (newState != cellInstances[i].state) {
            cellInstances[i].state = newState;
            // ��������� ���� ������
            Cell cell = pGameController->getGrid().getCell(x, y);
            cellInstances[i].color = cell.getColor();
            // ��������� ��� ������
            cellInstances[i].type = cell.getType();
            needsUpdate = true;
        }
    }

    // ���� ��������� ������ ����������, ��������� ����� ���������
    if (needsUpdate) {
        glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, cellInstances.size() * sizeof(CellInstance), cellInstances.data());
    }

    // ���������� ��������� ��������� ��� ��������� ������
    GL_CHECK(glUseProgram(shaderProgram));

    // ������������� uniform ���������� ��� ������� ��������, ���� � ������� ������
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint cellSizeLoc = glGetUniformLocation(shaderProgram, "cellSize");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
    GL_CHECK(glUniform1f(cellSizeLoc, pGameController->getCellSize()));

    // ����������� �������� ��� ������ �������� ������
    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);
    GL_CHECK(glEnableVertexAttribArray(0)); // ������� �������� (�������)
    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr));

    // ����������� �������� ��� �����������
    glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);
    GL_CHECK(glEnableVertexAttribArray(1)); // ������� ��������
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
    GL_CHECK(glVertexAttribDivisor(1, 1)); // ������ ������� ����� ���� �������

    GL_CHECK(glEnableVertexAttribArray(2)); // ��������� ������
    GL_CHECK(glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, state))));
    GL_CHECK(glVertexAttribDivisor(2, 1)); // ��������� ��� ������� ��������

    GL_CHECK(glEnableVertexAttribArray(3)); // ���� ������
    GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, color))));
    GL_CHECK(glVertexAttribDivisor(3, 1)); // ���� ��� ������� ��������

    GL_CHECK(glEnableVertexAttribArray(4)); // ��� ������
    GL_CHECK(glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, type))));
    GL_CHECK(glVertexAttribDivisor(4, 1)); // ��� ��� ������� ��������

    // ��������� ������ � �������������� �����������
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, cellInstances.size()));

    // ��������� ������������� ��������� ����� ���������
    GL_CHECK(glDisableVertexAttribArray(0));
    GL_CHECK(glDisableVertexAttribArray(1));
    GL_CHECK(glDisableVertexAttribArray(2));
    GL_CHECK(glDisableVertexAttribArray(3));
    GL_CHECK(glDisableVertexAttribArray(4));
}

void Renderer::DrawDebugOverlay() {
    if (!pGameController || !pGameController->isSimulationRunning()) return; // ��������� ��������
    glDisable(GL_DEPTH_TEST); // ��������� ���� ������� ��� �������

    GL_CHECK(glUseProgram(debugOverlayShaderProgram));

    // ������������� ���� ������������
    // ��������, ������� ����
    GLfloat redColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    GLuint overlayColorLocation = glGetUniformLocation(debugOverlayShaderProgram, "overlayColor");
    GL_CHECK(glUniform4fv(overlayColorLocation, 1, redColor));

    GL_CHECK(glBindVertexArray(debugOverlayVAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3)); // ������ �����������
    GL_CHECK(glBindVertexArray(0));

    glEnable(GL_DEPTH_TEST); // �������� ������� ���� ������� ����� ��������� �������
}

void Renderer::OnWindowResize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    glViewport(0, 0, width, height);
    // ��������� �������� ������
    camera.SetProjection(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f);
    UpdateDebugOverlayPosition();
}

void Renderer::MoveCamera(float dx, float dy, float dz) {
    // ��������� ������� ������
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
    vec3 color = vInstanceState > 0.5 ? vInstanceColor : vec3(0.1, 0.1, 0.1); // ������������� ����������� ����� ��� ����� ������
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
out float isMajorLine; // �������� �������� ��� ������������ �������
void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    // ������: ������������, �������� �� ����� "�������"
    isMajorLine = mod(aPos.x, 5.0) == 0.0 || mod(aPos.y, 5.0) == 0.0 ? 1.0 : 0.0;
}
        )";
    //std::string gridFragmentShaderSource = LoadShaderSource("./glsl/grid_fragment_shader.glsl");
//    const std::string gridFragmentShaderSource = R"(
//#version 330 core
//out vec4 FragColor;
//void main()
//{
//    FragColor = vec4(0.3, 0.3, 0.4, 1.0); // ����� ���� ��� �����
//}
//        )";
    const std::string gridFragmentShaderSource = R"(
#version 330 core
in float isMajorLine;
out vec4 FragColor;
void main()
{
    if(isMajorLine > 0.25) {
        FragColor = vec4(0.6, 0.6, 0.7, 1.0); // ������� ���� ��� "�������" �����
    } else {
        FragColor = vec4(0.3, 0.3, 0.4, 1.0); // ����� ���� ��� ������� �����
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
        std::cout << "�� ������� ��������� ���� ��������� ���������: " << filename << std::endl;
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
    // �������� � ���������� ��������
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

    // ����� �������� � �������� ��������� ���������
    GLuint overlayColorLocation = glGetUniformLocation(debugOverlayShaderProgram, "overlayColor");
    if (overlayColorLocation == -1) {
        std::cout << "�� ������� ����� uniform ���������� 'overlayColor' � �������" << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ������������� VAO � VBO ��� ���������� ������������ � ������ ������ ����
    float xOffset = 0.95f; // �������� �� X ��� ������� ������� ����
    float yOffset = -0.95f; // �������� �� Y ��� ������� ������� ����
    float size = 0.05f; // ������ ������������ � NDC

    // ���������� ������ ��� ������������, ������������� ������ �������� ������
    float vertices[] = {
        xOffset - size, yOffset,       // ����� �������
        xOffset, yOffset + size,       // ������� �������
        xOffset, yOffset               // ������ �������
    };

    glGenVertexArrays(1, &debugOverlayVAO);
    glGenBuffers(1, &debugOverlayVBO);

    glBindVertexArray(debugOverlayVAO);

    glBindBuffer(GL_ARRAY_BUFFER, debugOverlayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // �������
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    UpdateDebugOverlayPosition();
}

void Renderer::UpdateDebugOverlayPosition() {
    // ������� ������ VBO, ���� �� ����������
    if (debugOverlayVBO != 0) {
        glDeleteBuffers(1, &debugOverlayVBO);
    }

    // ������ ������������ � ��������
    const int triangleSizeInPixels = 20; // ��������, 20 ��������

    // ��������� �������� � ������ � ��������������� �����������
    float xOffset = (2.0f * (width - triangleSizeInPixels) / width) - 1.0f; // ������ ���� - ������ ������������
    float yOffset = (2.0f * (triangleSizeInPixels) / height) - 1.0f; // ������ ���� + ������ ������������
    float sizeX = (2.0f * triangleSizeInPixels / width); // ������ �� X � ��������������� �����������
    float sizeY = (2.0f * triangleSizeInPixels / height); // ������ �� Y � ��������������� �����������

    float vertices[] = {
        xOffset, yOffset + sizeY, // ������� �������
        xOffset - sizeX, yOffset, // ����� ������ �������
        xOffset, yOffset          // ������ ������ �������
    };

    glBindVertexArray(debugOverlayVAO);

    glGenBuffers(1, &debugOverlayVBO);
    glBindBuffer(GL_ARRAY_BUFFER, debugOverlayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // �������
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Renderer::RebuildGameField() {
    if (!pGameController) return;

    // ����������� ������ ������
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteBuffers(1, &gridVBO);

    InitializeVBOs(); // ������������� ������
}