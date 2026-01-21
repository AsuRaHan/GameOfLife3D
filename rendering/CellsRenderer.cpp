#include "CellsRenderer.h"
//#include "GLFunctions.h"  // Твои GL_CHECK и т.п.

CellsRenderer::CellsRenderer(const Camera& camera, ShaderManager& shaderManager)
    : BaseRenderer(camera, shaderManager),
      cellsVAO(0), cellsVBO(0), cellsIBO(0), cellInstanceVBO(0), cellShaderProgram(0),
      computeShaderProgram(0), visibleCellsBuffer(0), visibleColorsBuffer(0) {
    LoadShaders();
    LoadComputeShader();
}

CellsRenderer::~CellsRenderer() {
    glDeleteBuffers(1, &cellsVBO);
    glDeleteBuffers(1, &cellsIBO);
    glDeleteBuffers(1, &cellInstanceVBO);
    glDeleteVertexArrays(1, &cellsVAO);
    if (visibleCellsBuffer != 0) {
        glDeleteBuffers(1, &visibleCellsBuffer);
    }
    if (visibleColorsBuffer != 0) {
        glDeleteBuffers(1, &visibleColorsBuffer);
    }
}

void CellsRenderer::Initialize() {
    if (!pGameController) return;
    glGenVertexArrays(1, &cellsVAO);
    glBindVertexArray(cellsVAO);

    // Базовая геометрия куба (от -0.5 до 0.5)
    glGenBuffers(1, &cellsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);
    float s = 0.5f;
    float vertices[] = {
        -s, -s, -s,   0.0f, 0.0f, -1.0f,
         s, -s, -s,   0.0f, 0.0f, -1.0f,
         s,  s, -s,   0.0f, 0.0f, -1.0f,
        -s,  s, -s,   0.0f, 0.0f, -1.0f,

        -s, -s,  s,   0.0f, 0.0f,  1.0f,
         s, -s,  s,   0.0f, 0.0f,  1.0f,
         s,  s,  s,   0.0f, 0.0f,  1.0f,
        -s,  s,  s,   0.0f, 0.0f,  1.0f,

        -s, -s, -s,  -1.0f, 0.0f,  0.0f,
        -s, -s,  s,  -1.0f, 0.0f,  0.0f,
        -s,  s,  s,  -1.0f, 0.0f,  0.0f,
        -s,  s, -s,  -1.0f, 0.0f,  0.0f,

         s, -s, -s,   1.0f, 0.0f,  0.0f,
         s, -s,  s,   1.0f, 0.0f,  0.0f,
         s,  s,  s,   1.0f, 0.0f,  0.0f,
         s,  s, -s,   1.0f, 0.0f,  0.0f,

        -s, -s, -s,   0.0f, -1.0f, 0.0f,
         s, -s, -s,   0.0f, -1.0f, 0.0f,
         s, -s,  s,   0.0f, -1.0f, 0.0f,
        -s, -s,  s,   0.0f, -1.0f, 0.0f,

        -s,  s, -s,   0.0f,  1.0f, 0.0f,
         s,  s, -s,   0.0f,  1.0f, 0.0f,
         s,  s,  s,   0.0f,  1.0f, 0.0f,
        -s,  s,  s,   0.0f,  1.0f, 0.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Индексы куба
    glGenBuffers(1, &cellsIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cellsIBO);
    unsigned int indices[] = {
        0, 1, 2, 0, 2, 3,     // front
        4, 5, 6, 4, 6, 7,     // back
        8, 9, 10, 8, 10, 11,  // left
        12, 13, 14, 12, 14, 15, // right
        16, 17, 18, 16, 18, 19, // bottom
        20, 21, 22, 20, 22, 23  // top
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Буфер для инстансов
    glGenBuffers(1, &cellInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cellInstanceVBO);
    // Резервируем начальный размер
    glBufferData(GL_ARRAY_BUFFER, sizeof(CellInstance) * 1000, nullptr, GL_DYNAMIC_DRAW);

    // Instance position (vec4: x,y,z,pad)
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // Instance color (vec4)
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}

void CellsRenderer::Draw() {
    if (!pGameController) return;
    int minX, minY, maxX, maxY;
    GetVisibleGridBounds(minX, minY, maxX, maxY);
    int width = maxX - minX + 1;
    int height = maxY - minY + 1;
    size_t totalCells = width * height;

    // Проверяем, что compute shader успешно создан
    if (computeShaderProgram == 0) {
        // Если нет, используем старый метод (CPU)
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
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, visibleCellInstances.size());
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
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalCells * sizeof(float) * 4, nullptr, GL_DYNAMIC_COPY);  // vec4 для pos (x,y,z,pad)
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
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            viewProj[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++) {
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
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, nullptr);  // vec4 (x,y,z,pad)
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    glBindBuffer(GL_ARRAY_BUFFER, visibleColorsBuffer);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    // Используем width * height вместо totalCells
    int instanceCount = width * height;
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, instanceCount);
    glBindVertexArray(0);
}

void CellsRenderer::Rebuild() {
    Initialize();
}

void CellsRenderer::LoadShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec4 aInstancePos;  // x,y,z,pad
        layout(location = 3) in vec4 aInstanceColor;

        uniform mat4 projection;
        uniform mat4 view;
        uniform float cellSize;

        out vec3 vNormal;
        out vec4 vColor;

void main() {
    // Масштабируем базовую геометрию куба
    vec3 scaledPos = aPos * cellSize * 0.5;  // куб от -0.5..0.5 → размер cellSize

    // Добавляем позицию инстанса + смещение в центр клетки
    vec3 worldPos = scaledPos + aInstancePos.xyz + vec3(cellSize * 0.5, cellSize * 0.5, 0.0);

    gl_Position = projection * view * vec4(worldPos, 1.0);
    vNormal = aNormal;
    vColor = aInstanceColor;
}
    )";
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 vNormal;
        in vec4 vColor;
        out vec4 FragColor;

        uniform vec3 u_lightDir = vec3(0.5, 0.8, 1.0);
        uniform float u_glow = 1.2;

        void main() {
            vec3 normal = normalize(vNormal);
            vec3 light = normalize(u_lightDir);
            float diff = max(dot(normal, light), 0.0);

            vec3 base = vColor.rgb * (0.3 + 0.7 * diff);
            FragColor = vec4(base * u_glow, vColor.a);
        }
    )";
    cellShaderProgram = LoadShaderProgram("cells3dShader", vertexShaderSource, fragmentShaderSource);
}

void CellsRenderer::UpdateVisibleCells() {
    int minX, minY, maxX, maxY;
    GetVisibleGridBounds(minX, minY, maxX, maxY);
    float cellSize = pGameController->getCellSize();
    visibleCellInstances.clear();
    std::vector<int> gridState;
    pGameController->getGPUAutomaton().GetGridState(gridState);
    std::vector<float> colors;
    pGameController->getGPUAutomaton().GetColorsBuf(colors);
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            int index = y * pGameController->getGridWidth() + x;
            if (gridState[index] > 0) { // Только живые клетки
                float worldX = x * cellSize;
                float worldY = y * cellSize;
                float worldZ = 0.0f;  // Z по умолчанию (можно доработать)
                float pad = 0.0f;
                float r = colors[index * 4 + 0];
                float g = colors[index * 4 + 1];
                float b = colors[index * 4 + 2];
                float a = colors[index * 4 + 3];
                visibleCellInstances.push_back({worldX, worldY, worldZ, pad, r, g, b, a});
            }
        }
    }
}

bool CellsRenderer::IsCellVisible(float worldX, float worldY) const {
    const float* view = camera.GetViewMatrix();
    const float* proj = camera.GetProjectionMatrix();
    float cellSize = pGameController->getCellSize();
    // Проверяем все четыре угла клетки
    float corners[4][2] = {
        {worldX, worldY}, // Левый нижний
        {worldX + cellSize, worldY}, // Правый нижний
        {worldX, worldY + cellSize}, // Левый верхний
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

void CellsRenderer::GetVisibleGridBounds(int& minX, int& minY, int& maxX, int& maxY) const {
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

void CellsRenderer::LoadComputeShader() {
    const char* computeShaderSource = R"(
        #version 430
        layout(local_size_x = 16, local_size_y = 16) in;
        layout(std430, binding = 0) buffer VisibleCells {
            vec4 visibleCells[];  // x,y,z,pad
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
        uniform ivec2 viewBounds; // minX, minY
        uniform ivec2 viewExtent; // width, height
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
            visibleCells[localIndex] = vec4(worldPos, 0.0, 0.0);  // z=0, pad=0
            visibleColors[localIndex] = colors[gridIndex];
        }
    )";
    shaderManager.loadComputeShader("visibleCellsComputeCub", computeShaderSource);
    shaderManager.linkComputeProgram("visibleCellsComputeCub", "visibleCellsComputeCub");
    computeShaderProgram = shaderManager.getProgram("visibleCellsComputeCub");

    if (computeShaderProgram == 0) {
        std::cerr << "[CellsRenderer] Failed to create compute shader program" << std::endl;
    }
}