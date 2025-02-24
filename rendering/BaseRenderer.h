#ifndef BASE_RENDERER_H
#define BASE_RENDERER_H

#include "Camera.h"
#include "../system/GLFunctions.h"
#include "../system/ShaderManager.h"
#include "../game/GameController.h"
#include "../mathematics/Vector3d.h"

#include <vector>
#include <iostream>


class BaseRenderer {
public:
    BaseRenderer(const Camera& camera, ShaderManager& shaderManager);
    virtual ~BaseRenderer();

    void SetGameController(GameController* gameController);
    virtual void Initialize() = 0; // Чисто виртуальный метод для инициализации
    virtual void Draw() = 0;       // Чисто виртуальный метод для отрисовки

protected:
    const Camera& camera;
    ShaderManager& shaderManager;
    GameController* pGameController;

    // Методы для работы с VAO/VBO
    void CreateBuffers(GLuint& vao, GLuint& vbo, const float* vertices, size_t size, const std::vector<std::pair<GLint, GLint>>& attribs);
    void DeleteBuffers(GLuint& vao, GLuint& vbo);

    // Методы для работы с шейдерами
    GLuint LoadShaderProgram(const std::string& programName, const std::string& vertexSource, const std::string& fragmentSource);
    GLint GetUniformLocation(GLuint program, const std::string& name);
    void SetCommonUniforms(GLuint program); // Установка projection и view

private:
    std::unordered_map<std::string, GLint> uniformCache; // Кэш униформов для каждого шейдера
};

#endif