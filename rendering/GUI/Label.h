#pragma once
#include "UIElement.h"
#include "../../mathematics/Vector3d.h"

class Label : public UIElement {
public:
    Label(float x, float y, const std::string& text, const Vector3d& color);
    void draw(GLuint shaderProgram) override;
    bool handleClick(float mouseX, float mouseY) override;
}; 