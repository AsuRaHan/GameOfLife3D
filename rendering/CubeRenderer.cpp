#include "CubeRenderer.h"

CubeRenderer::CubeRenderer(ShaderManager& shaderManager) : shaderManager(shaderManager), pCamera(nullptr) {
    SetupCubeData();
    shaderManager.loadVertexShader("cubeVertexShader", R"(
    #version 330 core
    layout(location = 0) in vec3 aPos; 
    layout(location = 1) in vec3 aInstancePosition;
    layout(location = 2) in vec3 aInstanceColor;
    uniform mat4 projection;
    uniform mat4 view;
    uniform float scale;
    uniform vec3 cameraPosition;
    out vec3 vColor;
    out vec3 Normal;
    out vec3 FragPos;
    void main()
    {
        vec3 worldPosition = aInstancePosition + vec3(scale * 0.5, scale * 0.5, 0.0);
        vec3 scaledPosition = (aPos * scale * 0.8) + worldPosition;
        FragPos = scaledPosition;
        Normal = normalize(aPos);
        gl_Position = projection * view * vec4(scaledPosition, 1.0);
        vColor = aInstanceColor;
    }
    )");

    shaderManager.loadFragmentShader("cubeFragmentShader", R"(
    #version 330 core
    in vec3 vColor;
    in vec3 Normal;
    in vec3 FragPos;
    uniform vec3 cameraPosition;
    uniform vec3 lightColor;
    out vec4 FragColor;
    void main()
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(cameraPosition - FragPos); // направление света от камеры к фрагменту
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        vec3 result = (0.3 + diffuse) * vColor; // 0.3 - это амбиентный цвет
        FragColor = vec4(result, 1.0);
    }
    )");

    shaderManager.linkProgram("cubeShaderProgram", "cubeVertexShader", "cubeFragmentShader");
}

void CubeRenderer::SetCamera(const Camera& camera) {
    this->pCamera = &camera;
}

void CubeRenderer::SetupCubeData() {
    // Вершины куба
    //float vertices[] = {
    //    -0.5f, -0.5f, -0.5f,
    //     0.5f, -0.5f, -0.5f,
    //     0.5f,  0.5f, -0.5f,
    //    -0.5f,  0.5f, -0.5f,
    //    -0.5f, -0.5f,  0.5f,
    //     0.5f, -0.5f,  0.5f,
    //     0.5f,  0.5f,  0.5f,
    //    -0.5f,  0.5f,  0.5f
    //};

    //// Индексы для построения куба
    //unsigned int indices[] = {
    //    0, 1, 2, 2, 3, 0, // front
    //    1, 5, 6, 6, 2, 1, // right
    //    7, 6, 5, 5, 4, 7, // back
    //    4, 0, 3, 3, 7, 4, // left
    //    4, 5, 1, 1, 0, 4, // bottom
    //    3, 2, 6, 6, 7, 3  // top
    //};
    float vertices[] = {
    -0.5f, -0.5f, -0.55f,  // 0 - изменена Z-координата
     0.5f, -0.5f, -0.55f,  // 1 - изменена Z-координата
     0.5f,  0.5f, -0.55f,  // 2 - изменена Z-координата
    -0.5f,  0.5f, -0.55f,  // 3 - изменена Z-координата
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f
    };

    // Индексы остаются теми же, так как порядок вершин не изменился
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0, // front
        1, 5, 6, 6, 2, 1, // right
        7, 6, 5, 5, 4, 7, // back
        4, 0, 3, 3, 7, 4, // left
        4, 5, 1, 1, 0, 4, // bottom
        3, 2, 6, 6, 7, 3  // top
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Вершинные атрибуты для куба
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Убедитесь, что instanceVBO также правильно настроен
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void CubeRenderer::Render(const std::vector<Vector3d>& positions, const std::vector<Vector3d>& colors, float cellSize) {
    if (!pCamera) return;

    GLuint shaderProgram = shaderManager.getProgram("cubeShaderProgram");
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    // Настройка атрибутов для инстансинга
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(Vector3d) * 2, NULL, GL_STATIC_DRAW);
    for (size_t i = 0; i < positions.size(); ++i) {
        glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(Vector3d) * 2, sizeof(Vector3d), &positions[i]);
        glBufferSubData(GL_ARRAY_BUFFER, (i * sizeof(Vector3d) * 2) + sizeof(Vector3d), sizeof(Vector3d), &colors[i]);
    }

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector3d), (void*)0);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector3d), (void*)sizeof(Vector3d));
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(1, 1); // Это говорит OpenGL использовать следующий атрибут для каждого инстанса
    glVertexAttribDivisor(2, 1);

    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint scaleLoc = glGetUniformLocation(shaderProgram, "scale");
    GLint cameraPositionLoc = glGetUniformLocation(shaderProgram, "cameraPosition");
    GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, pCamera->GetProjectionMatrix());
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, pCamera->GetViewMatrix());
    glUniform1f(scaleLoc, cellSize);

    // Получаем позицию камеры
    float posX, posY, posZ;
    pCamera->GetPosition(posX, posY, posZ);
    glUniform3f(cameraPositionLoc, posX, posY, posZ);

    // Установка цвета света
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

    //glDrawElementsInstanced(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0, positions.size());
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, positions.size());

    glBindVertexArray(0);
}