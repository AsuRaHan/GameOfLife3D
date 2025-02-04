#pragma once
#ifndef BUTTON_H_
#define BUTTON_H_


#include "../../system/GLFunctions.h"
#include "UIElement.h"
#include <functional>
#include "../../mathematics/Vector3d.h"


class Button : public UIElement {
private:
    std::function<void()> onClick;
    bool isHovered;

public:
    Button(float x, float y, float width, float height, 
           const std::string& text, const Vector3d& color);
    virtual ~Button() = default;
    void draw(GLuint shaderProgram) override;
    bool handleClick(float mouseX, float mouseY) override;
    void setOnClick(std::function<void()> callback) { onClick = callback; }
}; 
#endif // BUTTON_H_