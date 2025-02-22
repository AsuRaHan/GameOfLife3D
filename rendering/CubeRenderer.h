#pragma once
#ifndef CUBERENDERER_H_
#define CUBERENDERER_H_

#include "BaseRenderer.h"

class CubeRenderer : public BaseRenderer {
public:
    CubeRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~CubeRenderer() override;

    void Initialize() override;
    void Draw() override;

private:
    GLuint VAO, VBO, EBO, instanceVBO;
    GLuint cubeShaderProgram;

    void SetupCubeData();
    void LoadCubeShaders();
};

#endif // CUBERENDERER_H_