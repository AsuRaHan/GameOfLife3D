#include "Label.h"

Label::Label(float x, float y, const std::string& text, const Vector3d& color)
    : UIElement(x, y, 0.1f, 0.05f, text, color) {}

void Label::draw(GLuint shaderProgram) {
    if (!isVisible) return;

    GL_CHECK(glUseProgram(shaderProgram));
    
    // Установка позиции и размера
    GLint posLoc = glGetUniformLocation(shaderProgram, "uPosition");
    GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
    GL_CHECK(glUniform2f(posLoc, x, y));
    GL_CHECK(glUniform2f(scaleLoc, width, height));
    
    // Установка цвета
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    GL_CHECK(glUniform3f(colorLoc, color.X(), color.Y(), color.Z()));
    
    // Отрисовка
    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
    GL_CHECK(glBindVertexArray(0));
}

bool Label::handleClick(float mouseX, float mouseY) {
    return false;  // Метки не обрабатывают клики
} 