#include "ChunkedCellsRenderer.h"

ChunkedCellsRenderer::ChunkedCellsRenderer(const Camera& camera, ShaderManager& shaderManager)
    : BaseRenderer(camera, shaderManager), chunkSize(100) {
    SetupShaders();
}

ChunkedCellsRenderer::~ChunkedCellsRenderer() {
    for (auto& chunk : chunks) {
        glDeleteVertexArrays(1, &chunk.vao);
        glDeleteBuffers(1, &chunk.vbo);
        glDeleteBuffers(1, &chunk.instanceVBO);
        glDeleteBuffers(1, &chunk.colorVBO);
    }
}

void ChunkedCellsRenderer::Initialize() {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();

    chunks.clear();
    for (int y = 0; y < gridHeight; y += chunkSize) {
        for (int x = 0; x < gridWidth; x += chunkSize) {
            CreateChunk(x, y);
        }
    }
}

void ChunkedCellsRenderer::SetupShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 vertexPos;
        layout(location = 1) in vec2 instancePos;
        layout(location = 2) in vec4 instanceColor;
        uniform mat4 projection;
        uniform mat4 view;
        uniform float cellSize;
        out vec4 fragColor;
        void main() {
            vec2 scaledPos = vertexPos * cellSize;
            vec2 worldPos = scaledPos + instancePos;
            gl_Position = projection * view * vec4(worldPos, 0.0, 1.0);
            fragColor = instanceColor;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec4 fragColor;
        out vec4 FragColor;
        void main() {
            FragColor = fragColor;
        }
    )";

    cellShaderProgram = LoadShaderProgram("cellShader", vertexShaderSource, fragmentShaderSource);
}

void ChunkedCellsRenderer::CreateChunk(int gridX, int gridY) {
    Chunk chunk;
    chunk.gridX = gridX;
    chunk.gridY = gridY;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    chunk.width = chunkSize;
    chunk.height = chunkSize;
    if (gridX + chunkSize > gridWidth) {
        chunk.width = gridWidth - gridX;
    }
    if (gridY + chunkSize > gridHeight) {
        chunk.height = gridHeight - gridY;
    }

    glGenVertexArrays(1, &chunk.vao);
    glGenBuffers(1, &chunk.vbo);
    glGenBuffers(1, &chunk.instanceVBO);
    glGenBuffers(1, &chunk.colorVBO); // Новый буфер для цветов

    glBindVertexArray(chunk.vao);

    // VBO: базовая форма клетки (как у тебя)
    glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo);
    float vertices[] = {
        0.1f, 0.1f,
        0.9f, 0.1f,
        0.9f, 0.9f,
        0.1f, 0.9f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // instanceVBO: позиции клеток
    glBindBuffer(GL_ARRAY_BUFFER, chunk.instanceVBO);
    std::vector<float> instancePositions;
    instancePositions.reserve(chunk.width * chunk.height * 2);
    for (int y = 0; y < chunk.height; y++) {
        for (int x = 0; x < chunk.width; x++) {
            float worldX = (gridX + x) * cellSize;
            float worldY = (gridY + y) * cellSize;
            instancePositions.push_back(worldX);
            instancePositions.push_back(worldY);
        }
    }
    glBufferData(GL_ARRAY_BUFFER, instancePositions.size() * sizeof(float), instancePositions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    // colorVBO: цвета из colorsBuffer для каждой клетки
    glBindBuffer(GL_ARRAY_BUFFER, chunk.colorVBO);
    std::vector<float> colorData;
    colorData.reserve(chunk.width * chunk.height * 4); // 4 float на цвет (RGBA)
    GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
    if (colorsBuffer != 0) {
        // Проверка состояния буфера
        GLint bufferSize;
        glBindBuffer(GL_COPY_READ_BUFFER, colorsBuffer);
        glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &bufferSize);
        std::cout << "colorsBuffer size: " << bufferSize << " bytes" << std::endl;

        // Сильная синхронизация
        glFinish(); // Ждем завершения всех операций OpenGL
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_BUFFER_READ_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

        // Проверка, что буфер содержит данные
        bool bufferHasData = false;
        for (int y = 0; y < chunk.height && !bufferHasData; y++) {
            for (int x = 0; x < chunk.width && !bufferHasData; x++) {
                int bufferIndex = ((gridY + y) * gridWidth + (gridX + x)) * 4;
                if (bufferIndex + 3 < bufferSize / sizeof(float)) { // Убедимся, что индекс в пределах
                    float color[4];
                    glGetBufferSubData(GL_COPY_READ_BUFFER, bufferIndex, 4 * sizeof(float), color);
                    if (color[0] != 0.0f || color[1] != 0.0f || color[2] != 0.0f || color[3] != 0.0f) {
                        bufferHasData = true;
                    }
                }
            }
        }

        if (bufferHasData) {
            glBindBuffer(GL_COPY_READ_BUFFER, colorsBuffer);
            for (int y = 0; y < chunk.height; y++) {
                for (int x = 0; x < chunk.width; x++) {
                    int bufferIndex = ((gridY + y) * gridWidth + (gridX + x)) * 4; // Индекс в colorsBuffer
                    float color[4];
                    glGetBufferSubData(GL_COPY_READ_BUFFER, bufferIndex, 4 * sizeof(float), color);
                    // Отладка
                    if (gridX == 0 && gridY == 0 && x < 2 && y < 2) {
                        std::cout << "Color at (" << x << ", " << y << "): R=" << std::fixed << std::setprecision(2)
                            << color[0] << ", G=" << color[1] << ", B=" << color[2] << ", A=" << color[3] << std::endl;
                    }
                    colorData.push_back(color[0]); // R
                    colorData.push_back(color[1]); // G
                    colorData.push_back(color[2]); // B
                    colorData.push_back(color[3]); // A
                }
            }
            glBindBuffer(GL_COPY_READ_BUFFER, 0);
        }
        else {
            // Если буфер пуст, заполняем черным цветом
            std::cout << "Warning: colorsBuffer contains no data, filling with black" << std::endl;
            colorData.resize(chunk.width * chunk.height * 4, 0.0f); // Черный цвет
        }
    }
    else {
        // Если colorsBuffer не инициализирован, заполняем черным цветом
        std::cout << "Warning: colorsBuffer is not initialized, filling with black" << std::endl;
        colorData.resize(chunk.width * chunk.height * 4, 0.0f); // Черный цвет
    }
    glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(float), colorData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
    chunks.push_back(chunk);
}

void ChunkedCellsRenderer::Draw() {
    if (!pGameController) return;

    glUseProgram(cellShaderProgram);
    SetCommonUniforms(cellShaderProgram);

    // Устанавливаем cellSize
    GLint cellSizeLoc = GetUniformLocation(cellShaderProgram, "cellSize");
    glUniform1f(cellSizeLoc, pGameController->getCellSize());

    for (const auto& chunk : chunks) {
        if (IsChunkInView(chunk)) {
            RenderChunk(chunk);
        }
    }
}

void ChunkedCellsRenderer::RenderChunk(const Chunk& chunk) {
    glBindVertexArray(chunk.vao);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, chunk.width * chunk.height);
    glBindVertexArray(0);
}

bool ChunkedCellsRenderer::IsChunkInView(const Chunk& chunk) const {
    float cellSize = pGameController->getCellSize();
    float minX = chunk.gridX * cellSize;
    float minY = chunk.gridY * cellSize;
    float maxX = (chunk.gridX + chunk.width) * cellSize;
    float maxY = (chunk.gridY + chunk.height) * cellSize;

    const float* projection = camera.GetProjectionMatrix();
    const float* view = camera.GetViewMatrix();

    // Проверяем углы bounding box чанка
    float corners[8][4] = {
        {minX, minY, 0.0f, 1.0f}, {maxX, minY, 0.0f, 1.0f},
        {minX, maxY, 0.0f, 1.0f}, {maxX, maxY, 0.0f, 1.0f},
        {minX, minY, 0.0f, 1.0f}, {maxX, minY, 0.0f, 1.0f},
        {minX, maxY, 0.0f, 1.0f}, {maxX, maxY, 0.0f, 1.0f}
    };

    for (int i = 0; i < 4; i++) {
        float x = corners[i][0] * view[0] + corners[i][1] * view[4] + corners[i][2] * view[8] + corners[i][3] * view[12];
        float y = corners[i][0] * view[1] + corners[i][1] * view[5] + corners[i][2] * view[9] + corners[i][3] * view[13];
        float z = corners[i][0] * view[2] + corners[i][1] * view[6] + corners[i][2] * view[10] + corners[i][3] * view[14];
        float w = corners[i][0] * view[3] + corners[i][1] * view[7] + corners[i][2] * view[11] + corners[i][3] * view[15];

        float px = x * projection[0] + y * projection[4] + z * projection[8] + w * projection[12];
        float py = x * projection[1] + y * projection[5] + z * projection[9] + w * projection[13];
        float pw = x * projection[3] + y * projection[7] + z * projection[11] + w * projection[15];

        if (pw != 0.0f) {
            px /= pw;
            py /= pw;
        }

        if (px >= -1.0f && px <= 1.0f && py >= -1.0f && py <= 1.0f) {
            return true;
        }
    }
    return false;
}