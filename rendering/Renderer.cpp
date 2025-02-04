#include "Renderer.h"
#include <algorithm> // ��� std::min

Renderer::Renderer(int width, int height)
    : width(width), height(height), camera(45.0f, static_cast<float>(width) / height, 0.1f, 1000000000.0f),
    pGameController(nullptr){
    SetupOpenGL();
    OnWindowResize(width, height);
    // ���������� UI ���������
    Button* startButton = new Button(10.0f, 50.0f, 100.0f, 30.0f, "Start", Vector3d(0.0f, 1.0f, 0.0f)); // ������� ����
    ui.addElement(startButton);

    Button* stopButton = new Button(120.0f, 50.0f, 100.0f, 30.0f, "Stop", Vector3d(1.0f, 0.0f, 0.0f)); // ������� ����
    ui.addElement(stopButton);

    //TextLabel* title = new TextLabel(width / 2 - 100.0f, height - 30.0f, "Game of Life 3D", Vector3d(1.0f, 1.0f, 0.0f)); // ������ ����
    //ui.addElement(title);
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
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // ������ ���
    GL_CHECK(glEnable(GL_DEPTH_TEST)); // �������� ���� �������
    GL_CHECK(glViewport(0, 0, width, height));
}

void Renderer::InitializeVBOs() {
    if (!pGameController) return;
    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // ������������� VBO ��� ������
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
        centerX + (0.1f - centerX) * (1 - scale_factor), centerY + (0.1f - centerY) * (1 - scale_factor), // ������� 1
        centerX + (0.5f - centerX) * (1 - scale_factor), centerY + (0.0f - centerY) * (1 - scale_factor), // ������� 5
        centerX + (0.9f - centerX) * (1 - scale_factor), centerY + (0.1f - centerY) * (1 - scale_factor), // ������� 2
        centerX + (1.0f - centerX) * (1 - scale_factor), centerY + (0.5f - centerY) * (1 - scale_factor), // ������� 6
        centerX + (0.9f - centerX) * (1 - scale_factor), centerY + (0.9f - centerY) * (1 - scale_factor), // ������� 3
        centerX + (0.5f - centerX) * (1 - scale_factor), centerY + (1.0f - centerY) * (1 - scale_factor), // ������� 7
        centerX + (0.1f - centerX) * (1 - scale_factor), centerY + (0.9f - centerY) * (1 - scale_factor), // ������� 4
        centerX + (0.0f - centerX) * (1 - scale_factor), centerY + (0.5f - centerY) * (1 - scale_factor)  // ������� 8
    };

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    // ������������� VBO ��� ������ ��������
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

    // �������� �����
    for (int y = 0; y <= gridHeight; ++y) {
        for (int x = 0; x <= gridWidth; ++x) {
            gridVertices.push_back(x * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
            gridVertices.push_back((x + 1) * cellSize); gridVertices.push_back(y * cellSize); gridVertices.push_back(0.0f);
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
    GL_CHECK(glGenVertexArrays(1, &gridVAO));
    GL_CHECK(glGenBuffers(1, &gridVBO));
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, gridVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));

    // �������� VAO � VBO ��� "�������" �����
    GLuint majorGridVAO, majorGridVBO;
    GL_CHECK(glGenVertexArrays(1, &majorGridVAO));
    GL_CHECK(glGenBuffers(1, &majorGridVBO));
    GL_CHECK(glBindVertexArray(majorGridVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, majorGridVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, majorGridVertices.size() * sizeof(float), majorGridVertices.data(), GL_STATIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));

    // ������ ��� ����� ����� ������������ ��� VAO � ������ DrawGrid:
    GL_CHECK(glBindVertexArray(gridVAO));
    GL_CHECK(glDrawArrays(GL_LINES, 0, gridVertices.size() / 3));
    GL_CHECK(glBindVertexArray(majorGridVAO));
    GL_CHECK(glDrawArrays(GL_LINES, 0, majorGridVertices.size() / 3));
    GL_CHECK(glBindVertexArray(0));
}

void Renderer::Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ��������� ����� � ������
    if(showGrid)DrawGrid();

    DrawCells();

    ui.draw();

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

    // ��������� ������ � �������
    for (size_t i = 0; i < cellInstances.size(); ++i) {
        int x = i % pGameController->getGridWidth();
        int y = i / pGameController->getGridWidth();
        Cell cell = pGameController->getGrid().getCell(x, y);
        cellInstances[i].color = cell.getColor(); // ��������� ������ ����
    }

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (ptr) {
        memcpy(ptr, cellInstances.data(), cellInstances.size() * sizeof(CellInstance));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    GL_CHECK(glUseProgram(shaderProgram));

    // ������������� uniform ����������
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint cellSizeLoc = glGetUniformLocation(shaderProgram, "cellSize");

    GL_CHECK(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix()));
    GL_CHECK(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix()));
    GL_CHECK(glUniform1f(cellSizeLoc, pGameController->getCellSize()));

    // ����������� �������� ��� ������ �������� ������
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellsVBO));
    GL_CHECK(glEnableVertexAttribArray(0)); // ������� �������� (�������)
    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr));

    // ����������� �������� ��� �����������
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO));
    GL_CHECK(glEnableVertexAttribArray(1)); // ������� ��������
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0));
    GL_CHECK(glVertexAttribDivisor(1, 1)); // ������ ������� ����� ���� �������

    GL_CHECK(glEnableVertexAttribArray(3)); // ���� ������
    GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(offsetof(CellInstance, color))));
    GL_CHECK(glVertexAttribDivisor(3, 1)); // ���� ��� ������� ��������

    // ��������� ������ � �������������� �����������
    //GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, cellInstances.size()));
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 8, cellInstances.size()));

    // ��������� ������������� ��������� ����� ���������
    GL_CHECK(glDisableVertexAttribArray(0));
    GL_CHECK(glDisableVertexAttribArray(1));
    GL_CHECK(glDisableVertexAttribArray(3));
}

void Renderer::DrawDebugOverlay() {
    if (!pGameController || !pGameController->isSimulationRunning()) return; // ��������� ��������
    GL_CHECK(glDisable(GL_DEPTH_TEST)); // ��������� ���� ������� ��� �������

    GL_CHECK(glUseProgram(debugOverlayShaderProgram));

    // ������������� ���� ������������
    // ��������, ������� ����
    GLfloat redColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    GLuint overlayColorLocation = glGetUniformLocation(debugOverlayShaderProgram, "overlayColor");
    GL_CHECK(glUniform4fv(overlayColorLocation, 1, redColor));

    GL_CHECK(glBindVertexArray(debugOverlayVAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3)); // ������ �����������
    GL_CHECK(glBindVertexArray(0));

    GL_CHECK(glEnable(GL_DEPTH_TEST)); // �������� ������� ���� ������� ����� ��������� �������
}

void Renderer::OnWindowResize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    GL_CHECK(glViewport(0, 0, width, height));
    // ��������� �������� ������
    camera.SetProjection(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f);
    UpdateDebugOverlayPosition();
    ui.OnWindowResize(width, height);
}

void Renderer::MoveCamera(float dx, float dy, float dz) {
    // ��������� ������� ������
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
out float isMajorLine; // �������� �������� ��� ������������ �������
void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    // ������: ������������, �������� �� ����� "�������"
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
        FragColor = vec4(0.6, 0.6, 0.7, 1.0); // ������� ���� ��� "�������" �����
    } else {
        FragColor = vec4(0.3, 0.3, 0.4, 1.0); // ����� ���� ��� ������� �����
    }
}
    )";
    shaderManager.loadVertexShader("gridVertexShader", gridVertexShaderSource.c_str());
    shaderManager.loadFragmentShader("gridFragmentShader", gridFragmentShaderSource.c_str());
    shaderManager.linkProgram("gridShaderProgram", "gridVertexShader", "gridFragmentShader");
    gridShaderProgram = shaderManager.getProgram("gridShaderProgram");

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