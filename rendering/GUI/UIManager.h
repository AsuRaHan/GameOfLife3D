#pragma once
#include <vector>
#include <memory>  // для std::unique_ptr
#include "UIElement.h"  // Добавьте эту строку
#include "../../system/GLFunctions.h"  // Добавьте эту строку

class UIManager {
private:
    std::vector<std::unique_ptr<UIElement>> elements;
    GLuint textShaderProgram;
    GLuint uiShaderProgram;

public:
    UIManager();
    void addElement(std::unique_ptr<UIElement> element);
    void draw();
    void handleClick(float mouseX, float mouseY);
    void updateLayout(int windowWidth, int windowHeight);
}; 