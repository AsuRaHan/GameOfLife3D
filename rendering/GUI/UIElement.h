#pragma once
#ifndef UIELEMENT_H_
#define UIELEMENT_H_

#include <string>
#include "../../mathematics/Vector3d.h"
#include "../../system/GLFunctions.h"

//class UIElement {
//public:
//    static int windowWidth;
//    static int windowHeight;
//
//    virtual ~UIElement() = default;
//    virtual void draw(GLuint shaderProgram) = 0;
//    virtual bool handleClick(float mouseX, float mouseY) = 0;
//
//    float getX() const { return x; }
//    float getY() const { return y; }
//    float getWidth() const { return width; }
//    float getHeight() const { return height; }
//    
//    void setPosition(float newX, float newY) {
//        x = newX;
//        y = newY;
//    }
//    
//    void setSize(float newWidth, float newHeight) {
//        width = newWidth;
//        height = newHeight;
//    }
//
//    void OnWindowResize(int newWidth, int newHeight);
//
//protected:
//    float x, y, width, height;
//    bool isVisible;
//    Vector3d color;
//    std::string text;
//    static GLuint VAO, VBO;
//    static const float vertices[8];
//    
//public:
//    UIElement(float x, float y, float width, float height, 
//             const std::string& text = "", const Vector3d& color = Vector3d(0.5f, 0.5f, 0.5f));
//    
//    static void screenToNormalized(int screenX, int screenY, int windowWidth, int windowHeight,
//                                 float& normalizedX, float& normalizedY);
//}; 

class UIElement {
public:
    virtual ~UIElement() = default;
    virtual void draw(GLuint shaderProgram, GLuint vao, GLuint vbo) = 0;
    virtual bool handleClick(float mouseX, float mouseY) = 0;

    float getX() const { return x; }
    float getY() const { return y; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    void setPosition(float newX, float newY) {
        x = newX;
        y = newY;
    }

    void setSize(float newWidth, float newHeight) {
        width = newWidth;
        height = newHeight;
    }

protected:
    float x, y, width, height;
    bool isVisible;
    Vector3d color;
    std::string text;
    static GLuint vao, vbo;
    static int windowWidth, windowHeight;

public:
    UIElement(float x, float y, float width, float height,
        const std::string& text = "", const Vector3d& color = Vector3d(0.5f, 0.5f, 0.5f));

    static void screenToNormalized(int screenX, int screenY, int windowWidth, int windowHeight,
        float& normalizedX, float& normalizedY);

    void OnWindowResize(int newWidth, int newHeight);
};
#endif // UIELEMENT_H_