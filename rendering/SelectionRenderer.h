#pragma once
#ifndef SELECTIONRENDERER_H_
#define SELECTIONRENDERER_H_

#include "BaseRenderer.h"
#include "../mathematics/Vector3d.h"

class SelectionRenderer : public BaseRenderer {
public:
    SelectionRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~SelectionRenderer();

    void SetSelectionStart(const Vector3d& start);
    void SetSelectionEnd(const Vector3d& end);
    void SetIsSelecting(bool isSelecting);

    void Initialize() override;
    void Draw() override;

private:
    void LoadShaders();

    GLuint VAO, VBO;
    Vector3d selectionStart;
    Vector3d selectionEnd;
    bool isSelecting;
    GLuint selectionShaderProgram;
};

#endif // SELECTIONRENDERER_H_