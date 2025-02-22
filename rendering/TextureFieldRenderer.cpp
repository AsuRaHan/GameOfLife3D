#include "TextureFieldRenderer.h"

TextureFieldRenderer::TextureFieldRenderer(const Camera& camera, ShaderManager& shaderManager)
    : BaseRenderer(camera, shaderManager), VAO(0), VBO(0), textureID(0), textureShaderProgram(0) {
}

TextureFieldRenderer::~TextureFieldRenderer() {
    DeleteBuffers(VAO, VBO);
    if (textureID) glDeleteTextures(1, &textureID);
}

void TextureFieldRenderer::Initialize() {
    if (!pGameController) return;

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    float vertices[] = {
        0.0f, 0.0f,                0.0f, 0.0f,
        gridWidth * cellSize, 0.0f, 1.0f, 0.0f,
        0.0f, gridHeight * cellSize, 0.0f, 1.0f,
        gridWidth * cellSize, gridHeight * cellSize, 1.0f, 1.0f
    };

    std::vector<std::pair<GLint, GLint>> attribs = { {2, 4}, {2, 4} }; // Позиция (2 float), текстурные координаты (2 float)
    CreateBuffers(VAO, VBO, vertices, sizeof(vertices), attribs);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_BUFFER, textureID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, pGameController->getGPUAutomaton().getColorsBuffer());
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    LoadTextureShaders();
}

void TextureFieldRenderer::Draw() {
    if (!pGameController) return;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, textureID);

    glUseProgram(textureShaderProgram);
    SetCommonUniforms(textureShaderProgram);

    GLint gridWidthLoc = GetUniformLocation(textureShaderProgram, "gridWidth");
    GLint gridHeightLoc = GetUniformLocation(textureShaderProgram, "gridHeight");
    GLint textureLoc = GetUniformLocation(textureShaderProgram, "textureSampler");
    glUniform1i(gridWidthLoc, pGameController->getGridWidth());
    glUniform1i(gridHeightLoc, pGameController->getGridHeight());
    glUniform1i(textureLoc, 0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void TextureFieldRenderer::LoadTextureShaders() {
    // ===================================================================================================================
    const char* vertexShaderSource = R"(
        #version 430 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        uniform mat4 projection;
        uniform mat4 view;
        out vec2 TexCoord;
        void main() {
            gl_Position = projection * view * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    // ===================================================================================================================
    const char* fragmentShaderSource = R"(
        #version 430 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform samplerBuffer textureSampler;
        uniform int gridWidth;
        uniform int gridHeight;
        void main() {
            int x = int(TexCoord.x * gridWidth);
            int y = int(TexCoord.y * gridHeight);
            int index = y * gridWidth + x;
            FragColor = texelFetch(textureSampler, index);
        }
    )";

    textureShaderProgram = LoadShaderProgram("textureFieldShaderProgram", vertexShaderSource, fragmentShaderSource);
}