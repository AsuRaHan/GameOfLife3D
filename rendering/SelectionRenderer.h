#pragma once
#ifndef SELECTIONRENDERER_H_
#define SELECTIONRENDERER_H_

#include "../system/GLFunctions.h"
#include "../system/ShaderManager.h"
#include "../mathematics/Vector3d.h"
#include "../game/GameController.h"
#include "Camera.h"
#include <vector>

class SelectionRenderer {
public:
    SelectionRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~SelectionRenderer();

    void SetSelectionStart(const Vector3d& start);
    void SetSelectionEnd(const Vector3d& end);
    void SetIsSelecting(bool isSelecting);
    void Draw();
    void SetGameController(GameController* gameController); // Добавляем сеттер
private:
    void SetupLineData();
    void LoadShaders();

    const Camera& camera;
    ShaderManager& shaderManager;
    GameController* pGameController = nullptr; // Инициализируем как nullptr

    GLuint VAO, VBO;
    Vector3d selectionStart;
    Vector3d selectionEnd;
    bool isSelecting;
    GLuint selectionShaderProgram;
};

#endif // SELECTIONRENDERER_H_