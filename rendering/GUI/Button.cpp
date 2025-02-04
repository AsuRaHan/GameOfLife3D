#include "Button.h"

Button::Button(float x, float y, float width, float height,
               const std::string& text, const Vector3d& color)
    : UIElement(x, y, width, height, text, color), isHovered(false) {}

void Button::draw(GLuint shaderProgram) {
    if (!isVisible) return;

    GL_CHECK(glUseProgram(shaderProgram));
    
    // Установка позиции и размера
    GLint posLoc = glGetUniformLocation(shaderProgram, "uPosition");
    GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
    GL_CHECK(glUniform2f(posLoc, x, y));
    GL_CHECK(glUniform2f(scaleLoc, width, height));
    
    // Установка цвета и состояния наведения
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    GLint hoverLoc = glGetUniformLocation(shaderProgram, "uHovered");
    GL_CHECK(glUniform3f(colorLoc, color.X(), color.Y(), color.Z()));
    GL_CHECK(glUniform1f(hoverLoc, isHovered ? 1.0f : 0.0f));
    
    // Отрисовка
    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
    GL_CHECK(glBindVertexArray(0));
}

bool Button::handleClick(float mouseX, float mouseY) {
    if (!isVisible) return false;
    
    bool wasClicked = UIElement::handleClick(mouseX, mouseY);
    if (wasClicked && onClick) {
        onClick();
    }
    return wasClicked;
} 