#include "UIElement.h"

// Инициализация статических членов
GLuint UIElement::VAO = 0;
GLuint UIElement::VBO = 0;

// Вершины для прямоугольника (квад)
const float UIElement::vertices[] = {
    -1.0f, -1.0f,  // Нижний левый
     1.0f, -1.0f,  // Нижний правый
     1.0f,  1.0f,  // Верхний правый
    -1.0f,  1.0f   // Верхний левый
};

UIElement::UIElement(float x, float y, float width, float height, 
                     const std::string& text, const Vector3d& color)
    : x(x), y(y), width(width), height(height), 
      isVisible(true), color(color), text(text) {
    
    // Инициализация VAO/VBO (только один раз)
    if (VAO == 0) {
        GL_CHECK(glGenVertexArrays(1, &VAO));
        GL_CHECK(glGenBuffers(1, &VBO));
        
        GL_CHECK(glBindVertexArray(VAO));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
        
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));
        
        GL_CHECK(glBindVertexArray(0));
    }
}

bool UIElement::handleClick(float mouseX, float mouseY) {
    return mouseX >= x - width/2 && mouseX <= x + width/2 &&
           mouseY >= y - height/2 && mouseY <= y + height/2;
}

void UIElement::screenToNormalized(int screenX, int screenY, int windowWidth, int windowHeight,
                                 float& normalizedX, float& normalizedY) {
    normalizedX = (2.0f * screenX) / windowWidth - 1.0f;
    normalizedY = 1.0f - (2.0f * screenY) / windowHeight;
} 