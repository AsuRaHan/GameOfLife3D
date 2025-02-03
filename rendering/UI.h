#pragma once

#include <vector>
#include <string>
#include "../system/GLFunctions.h"
#include "../mathematics/Vector3d.h"

class UIElement {
public:
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual void OnWindowResize(int newWidth, int newHeight) = 0; // Добавляем виртуальный метод
    virtual ~UIElement() = default;
};

class Button : public UIElement {
public:
    Button(float x, float y, float width, float height, const std::string& text, const Vector3d& color = Vector3d(1.0f, 0.5f, 0.2f));
    void draw() override;
    void update() override;
    void OnWindowResize(int newWidth, int newHeight) override; // Объявляем метод
    bool isClicked(int mouseX, int mouseY) const;
    float x, y, width, height;
    Vector3d color;

private:
    std::string text;
};

class TextLabel : public UIElement {
public:
    TextLabel(float x, float y, const std::string& text, const Vector3d& color = Vector3d(1.0f, 1.0f, 1.0f));
    void draw() override;
    void update() override;
    void OnWindowResize(int newWidth, int newHeight) override; // Объявляем метод
    float x, y;
    Vector3d color;

private:
    std::string text;
};

class UI {
public:
    UI();
    ~UI();
    void addElement(UIElement* element);
    void draw();
    void update();
    void OnWindowResize(int newWidth, int newHeight);
    static int width;
    static int height;
    static UI& getInstance();
    void setElementColor(const Vector3d& color);

private:
    std::vector<UIElement*> elements;
    GLuint shaderProgram;
    static GLuint vao;
    static GLuint vbo;
    void loadShaders();
    void prepareVAO();
    void setUniformColor(const Vector3d& color);

    friend class Button;
    friend class TextLabel;
};