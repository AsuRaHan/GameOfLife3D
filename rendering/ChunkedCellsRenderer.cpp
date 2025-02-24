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
        layout(location = 3) in vec4 instanceColor;
        
        uniform mat4 projection;
        uniform mat4 view;
        uniform float cellSize;
        
        out vec4 fragColor;
        
        void main() {
            vec2 worldPos = (vertexPos * cellSize) + instancePos;
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

    glBindVertexArray(chunk.vao);

    // VBO: базовая форма клетки
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

    // instanceVBO: позиции клеток в мировых координатах
    glBindBuffer(GL_ARRAY_BUFFER, chunk.instanceVBO);
    std::vector<float> instanceData;
    instanceData.reserve(chunk.width * chunk.height * 2);
    for (int y = 0; y < chunk.height; y++) {
        for (int x = 0; x < chunk.width; x++) {
            float worldX = (gridX + x) * cellSize;
            float worldY = (gridY + y) * cellSize;
            instanceData.push_back(worldX);
            instanceData.push_back(worldY);
        }
    }
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(float), instanceData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    // Добавляем атрибут для цветов (location = 3)
    GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
    chunks.push_back(chunk);
}






void ChunkedCellsRenderer::Draw() {
    if (!pGameController) return;

    glUseProgram(cellShaderProgram);
    SetCommonUniforms(cellShaderProgram);

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
    
    GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
    if (colorsBuffer != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
        int gridWidth = pGameController->getGridWidth();
        
        // Исправляем расчёт смещения и настройку атрибута
        size_t offset = (chunk.gridY * gridWidth + chunk.gridX) * sizeof(float) * 4;
        
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)offset);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);
    }
    
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

    float corners[4][4] = {
        {minX, minY, 0.0f, 1.0f},
        {maxX, minY, 0.0f, 1.0f},
        {minX, maxY, 0.0f, 1.0f},
        {maxX, maxY, 0.0f, 1.0f}
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