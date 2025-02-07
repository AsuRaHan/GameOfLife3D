#pragma once
#ifndef CUBERENDERER_H_
#define CUBERENDERER_H_

#include "../system/GLFunctions.h"
#include "../system/ShaderManager.h"
#include "../mathematics/Vector3d.h"
#include "Camera.h"
#include <vector>
#include <array>
#include <string>


class CubeRenderer {
public:
    CubeRenderer(ShaderManager& shaderManager);
    void Render(const std::vector<Vector3d>& positions, const std::vector<Vector3d>& colors, float cellSize);
    void SetCamera(const Camera& camera);
private:
    void SetupCubeData();
    ShaderManager& shaderManager;
    GLuint VAO, VBO, EBO, instanceVBO;
    GLuint instanceCount;
    const Camera* pCamera; // ”казатель на камеру
};

#endif // CUBERENDERER_H_