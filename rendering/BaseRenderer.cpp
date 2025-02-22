#include "BaseRenderer.h"

#include "BaseRenderer.h"
#include <iostream>

BaseRenderer::BaseRenderer(const Camera& camera, ShaderManager& shaderManager)
    : camera(camera), shaderManager(shaderManager), pGameController(nullptr) {
}

BaseRenderer::~BaseRenderer() {}

void BaseRenderer::SetGameController(GameController* gameController) {
    pGameController = gameController;
}

void BaseRenderer::CreateBuffers(GLuint& vao, GLuint& vbo, const float* vertices, size_t size, const std::vector<std::pair<GLint, GLint>>& attribs) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    for (size_t i = 0; i < attribs.size(); ++i) {
        glVertexAttribPointer(i, attribs[i].first, GL_FLOAT, GL_FALSE, attribs[i].second * sizeof(float), (void*)(i * attribs[i].first * sizeof(float)));
        glEnableVertexAttribArray(i);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BaseRenderer::DeleteBuffers(GLuint& vao, GLuint& vbo) {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    vao = 0;
    vbo = 0;
}

GLuint BaseRenderer::LoadShaderProgram(const std::string& programName, const std::string& vertexSource, const std::string& fragmentSource) {
    shaderManager.loadVertexShader(programName + "Vertex", vertexSource.c_str());
    shaderManager.loadFragmentShader(programName + "Fragment", fragmentSource.c_str());
    shaderManager.linkProgram(programName, programName + "Vertex", programName + "Fragment");
    return shaderManager.getProgram(programName);
}

GLint BaseRenderer::GetUniformLocation(GLuint program, const std::string& name) {
    std::string key = std::to_string(program) + "_" + name;
    auto it = uniformCache.find(key);
    if (it != uniformCache.end()) {
        return it->second;
    }
    GLint loc = glGetUniformLocation(program, name.c_str());
    uniformCache[key] = loc;
    return loc;
}

void BaseRenderer::SetCommonUniforms(GLuint program) {
    GLint projectionLoc = GetUniformLocation(program, "projection");
    GLint viewLoc = GetUniformLocation(program, "view");
    if (projectionLoc != -1) glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.GetProjectionMatrix());
    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.GetViewMatrix());
}