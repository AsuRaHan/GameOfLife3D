#include "CellsViewportRenderer.h"

CellsViewportRenderer::CellsViewportRenderer(const Camera& camera, ShaderManager& shaderManager)
    : BaseRenderer(camera, shaderManager),
    cellsVAO(0), cellsVBO(0), cellInstanceVBO(0), cellShaderProgram(0),
    computeShaderProgram(0), visibleCellsBuffer(0), visibleColorsBuffer(0) {
    LoadShaders();
    LoadComputeShader();
}

CellsViewportRenderer::~CellsViewportRenderer() {
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteVertexArrays(1, &cellsVAO);
    if (visibleCellsBuffer != 0) {
        glDeleteBuffers(1, &visibleCellsBuffer);
    }
    if (visibleColorsBuffer != 0) {
        glDeleteBuffers(1, &visibleColorsBuffer);
    }
}

void CellsViewportRenderer::Initialize() {
    if (!pGameController) return;

    glGenVertexArrays(1, &cellsVAO);
    glBindVertexArray(cellsVAO);

    // Базовая геометрия клетки
    glGenBuffers(1, &cellsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);
    float vertices[] = {
        0.1f, 0.1f,
        0.9f, 0.1f,
        0.9f, 0.9f,
        0.1f, 0.9f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Буфер для инстансов
    glGenBuffers(1, &cellInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);
    // Резервируем начальный размер
    glBufferData(GL_ARRAY_BUFFER, sizeof(CellInstance) * 1000, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}

void CellsViewportRenderer::Draw() {
    if (!pGameController) return;

    int minX, minY, maxX, maxY;
    GetVisibleGridBounds(minX, minY, maxX, maxY);
    int width = maxX - minX + 1;
    int height = maxY - minY + 1;
    size_t totalCells = width * height;

    // Проверяем, что compute shader успешно создан
    if (computeShaderProgram == 0) {
        // Если нет, используем старый метод
        UpdateVisibleCells();
        if (visibleCellInstances.empty()) return;

        glUseProgram(cellShaderProgram);
        SetCommonUniforms(cellShaderProgram);
        glUniform1f(GetUniformLocation(cellShaderProgram, "cellSize"), pGameController->getCellSize());

        glBindVertexArray(cellsVAO);

        // Обновляем позиции видимых клеток
        glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, visibleCellInstances.size() * sizeof(CellInstance), 
                    visibleCellInstances.data(), GL_DYNAMIC_DRAW);

        // Настраиваем атрибут цвета
        GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
        if (colorsBuffer != 0) {
            glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
            int gridWidth = pGameController->getGridWidth();
            size_t colorOffset = (minY * gridWidth + minX) * sizeof(float) * 4;
            
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)colorOffset);
            glEnableVertexAttribArray(3);
            glVertexAttribDivisor(3, 1);
        }

        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, visibleCellInstances.size());
        glBindVertexArray(0);
        return;
    }

    // Обновляем или создаем буферы если нужно
    if (visibleCellsBuffer == 0) {
        glGenBuffers(1, &visibleCellsBuffer);
        glGenBuffers(1, &visibleColorsBuffer);
    }

    // Настраиваем буферы для compute shader
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleCellsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalCells * sizeof(float) * 2, nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, visibleCellsBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleColorsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalCells * sizeof(float) * 4, nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, visibleColorsBuffer);

    // Привязываем буфер цветов из GPUAutomaton
    GLuint colorsBuffer = pGameController->getGPUAutomaton().getColorsBuffer();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, colorsBuffer);

    // Запускаем compute shader
    glUseProgram(computeShaderProgram);
    
    // Вычисляем матрицу viewProj = projection * view
    const float* view = camera.GetViewMatrix();
    const float* proj = camera.GetProjectionMatrix();
    float viewProj[16];
    
    // Умножение матриц 4x4
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            viewProj[i * 4 + j] = 0;
            for(int k = 0; k < 4; k++) {
                viewProj[i * 4 + j] += proj[i * 4 + k] * view[k * 4 + j];
            }
        }
    }

    // Устанавливаем униформы
    glUniformMatrix4fv(glGetUniformLocation(computeShaderProgram, "viewProj"), 1, GL_FALSE, viewProj);
    glUniform1f(glGetUniformLocation(computeShaderProgram, "cellSize"), pGameController->getCellSize());
    glUniform2i(glGetUniformLocation(computeShaderProgram, "gridSize"), 
                pGameController->getGridWidth(), pGameController->getGridHeight());
    glUniform2i(glGetUniformLocation(computeShaderProgram, "viewBounds"), minX, minY);
    glUniform2i(glGetUniformLocation(computeShaderProgram, "viewExtent"), width, height);

    // Запускаем вычисления
    glDispatchCompute((width + 15) / 16, (height + 15) / 16, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Рендеринг
    glUseProgram(cellShaderProgram);
    SetCommonUniforms(cellShaderProgram);
    glUniform1f(GetUniformLocation(cellShaderProgram, "cellSize"), pGameController->getCellSize());

    glBindVertexArray(cellsVAO);

    // Используем вычисленные позиции и цвета
    glBindBuffer(GL_ARRAY_BUFFER, visibleCellsBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ARRAY_BUFFER, visibleColorsBuffer);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    // Используем width * height вместо totalCells
    int instanceCount = width * height;
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, instanceCount);
    glBindVertexArray(0);
}

void CellsViewportRenderer::LoadShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aInstancePos;
        layout(location = 3) in vec4 aInstanceColor;
        
        uniform mat4 projection;
        uniform mat4 view;
        uniform float cellSize;
        
        out vec4 fragColor;
        
        void main() {
            // Сначала масштабируем базовую геометрию клетки
            vec2 scaledPos = aPos * cellSize;
            // Затем добавляем позицию инстанса
            vec2 worldPos = scaledPos + aInstancePos;
            gl_Position = projection * view * vec4(worldPos, 0.0, 1.0);
            fragColor = aInstanceColor;
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

    cellShaderProgram = LoadShaderProgram("viewportCellShader", vertexShaderSource, fragmentShaderSource);
}

void CellsViewportRenderer::UpdateVisibleCells() {
    int minX, minY, maxX, maxY;
    GetVisibleGridBounds(minX, minY, maxX, maxY);

    float cellSize = pGameController->getCellSize();
    int gridWidth = pGameController->getGridWidth();
    visibleCellInstances.clear();

    // Сохраняем информацию о каждой клетке
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float worldX = x * cellSize;
            float worldY = y * cellSize;
            visibleCellInstances.push_back({worldX, worldY});
        }
    }
}

bool CellsViewportRenderer::IsCellVisible(float worldX, float worldY) const {
    const float* view = camera.GetViewMatrix();
    const float* proj = camera.GetProjectionMatrix();
    float cellSize = pGameController->getCellSize();

    // Проверяем все четыре угла клетки
    float corners[4][2] = {
        {worldX, worldY},                     // Левый нижний
        {worldX + cellSize, worldY},          // Правый нижний
        {worldX, worldY + cellSize},          // Левый верхний
        {worldX + cellSize, worldY + cellSize} // Правый верхний
    };

    for (const auto& corner : corners) {
        float x = corner[0] * view[0] + corner[1] * view[4] + view[12];
        float y = corner[0] * view[1] + corner[1] * view[5] + view[13];
        float w = corner[0] * view[3] + corner[1] * view[7] + view[15];

        float px = x * proj[0] + y * proj[4] + w * proj[12];
        float py = x * proj[1] + y * proj[5] + w * proj[13];
        float pw = x * proj[3] + y * proj[7] + w * proj[15];

        if (pw != 0.0f) {
            px /= pw;
            py /= pw;
        }

        // Если хотя бы один угол клетки видим, считаем клетку видимой
        if (px >= -1.1f && px <= 1.1f && py >= -1.1f && py <= 1.1f) {
            return true;
        }
    }
    return false;
}

void CellsViewportRenderer::GetVisibleGridBounds(int& minX, int& minY, int& maxX, int& maxY) const {
    float cellSize = pGameController->getCellSize();
    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();

    float camX, camY, camZ;
    camera.GetPosition(camX, camY, camZ);

    float dirX, dirY, dirZ;
    camera.GetDirection(dirX, dirY, dirZ);

    float viewDistance = std::abs(camZ);
    float aspectRatio = camera.GetAspectRatio(); // Используем соотношение сторон из камеры
    
    // Вычисляем размеры видимой области с учетом FOV и соотношения сторон
    float viewHeight = 2.0f * viewDistance * std::tan(camera.GetFOV() * 0.5f * Camera::PI / 180.0f);
    float viewWidth = viewHeight * aspectRatio;

    // Добавляем запас для поворота камеры и зума
    float rotationMargin = std::sqrt(viewWidth * viewWidth + viewHeight * viewHeight) * 0.5f;
    
    // Расширяем границы
    minX = static_cast<int>((camX - rotationMargin) / cellSize) - 2;
    maxX = static_cast<int>((camX + rotationMargin) / cellSize) + 2;
    minY = static_cast<int>((camY - rotationMargin) / cellSize) - 2;
    maxY = static_cast<int>((camY + rotationMargin) / cellSize) + 2;

    // Ограничиваем границы размерами сетки
    minX = (minX < 0) ? 0 : (minX > gridWidth - 1) ? gridWidth - 1 : minX;
    maxX = (maxX < 0) ? 0 : (maxX > gridWidth - 1) ? gridWidth - 1 : maxX;
    minY = (minY < 0) ? 0 : (minY > gridHeight - 1) ? gridHeight - 1 : minY;
    maxY = (maxY < 0) ? 0 : (maxY > gridHeight - 1) ? gridHeight - 1 : maxY;
}

void CellsViewportRenderer::LoadComputeShader() {
    const char* computeShaderSource = R"(
        #version 430
        layout(local_size_x = 16, local_size_y = 16) in;

        layout(std430, binding = 0) buffer VisibleCells {
            vec2 visibleCells[];
        };

        layout(std430, binding = 1) buffer Colors {
            vec4 colors[];
        };

        layout(std430, binding = 2) buffer VisibleColors {
            vec4 visibleColors[];
        };

        uniform mat4 viewProj;
        uniform float cellSize;
        uniform ivec2 gridSize;
        uniform ivec2 viewBounds;  // minX, minY
        uniform ivec2 viewExtent;  // width, height

        bool isCellVisible(vec2 worldPos) {
            vec4 clipPos = viewProj * vec4(worldPos, 0.0, 1.0);
            vec3 ndcPos = clipPos.xyz / clipPos.w;
            return all(greaterThanEqual(ndcPos.xy, vec2(-1.1))) && 
                   all(lessThanEqual(ndcPos.xy, vec2(1.1)));
        }

        void main() {
            ivec2 id = ivec2(gl_GlobalInvocationID.xy);
            if (id.x >= viewExtent.x || id.y >= viewExtent.y) return;

            ivec2 gridPos = viewBounds + id;
            if (any(greaterThanEqual(gridPos, gridSize))) return;

            vec2 worldPos = vec2(gridPos) * cellSize;
            int gridIndex = gridPos.y * gridSize.x + gridPos.x;
            int localIndex = id.y * viewExtent.x + id.x;

            // Инициализируем значения по умолчанию
            visibleCells[localIndex] = worldPos;
            visibleColors[localIndex] = colors[gridIndex];
            
            // Проверка видимости больше не нужна, так как мы уже отфильтровали по границам
            // if (isCellVisible(worldPos)) {
            //     visibleCells[localIndex] = worldPos;
            //     visibleColors[localIndex] = colors[gridIndex];
            // }
        }
    )";

    shaderManager.loadComputeShader("visibleCellsCompute", computeShaderSource);
    shaderManager.linkComputeProgram("visibleCellsCompute", "visibleCellsCompute");
    computeShaderProgram = shaderManager.getProgram("visibleCellsCompute");
    
    if (computeShaderProgram == 0) {
        std::cerr << "[CellsViewportRenderer] Failed to create compute shader program" << std::endl;
    }
} 