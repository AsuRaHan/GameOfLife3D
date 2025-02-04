#pragma once
#ifndef UIMANAGER_H_
#define UIMANAGER_H_

#include <vector>
#include <memory>  // для std::unique_ptr
#include "UIElement.h"  // Добавьте эту строку
#include "../../system/GLFunctions.h"  // Добавьте эту строку
#include "../../system/ShaderManager.h"

class UIManager {
private:
    //std::vector<std::unique_ptr<UIElement>> elements;
    //GLuint textShaderProgram;
    //GLuint uiShaderProgram;
    std::vector<std::unique_ptr<UIElement>> elements;
    GLuint shaderProgram;
    GLuint vao, vbo;
    ShaderManager shaderManager;
    void loadShaders();
    void prepareVAO();

public:
    //UIManager();
    //void addElement(std::unique_ptr<UIElement> element);
    //void draw();
    //void handleClick(float mouseX, float mouseY);
    //void updateLayout(int windowWidth, int windowHeight);
    //void handleMouseMove(float x, float y);
    UIManager();
    void addElement(std::unique_ptr<UIElement> element);
    void draw();
    void updateLayout(int windowWidth, int windowHeight);
    void handleMouseMove(float x, float y);
    void handleClick(float mouseX, float mouseY);
    void setElementColor(const Vector3d& color);
}; 
#endif // UIMANAGER_H_