#include "Label.h"

Label::Label(float x, float y, const std::string& text, const Vector3d& color)
    : UIElement(x, y, 100.0f, 50.0f, text, color) { // Примерные размеры для текста
}

void Label::draw(GLuint shaderProgram, GLuint vao, GLuint vbo) {
    glDisable(GL_DEPTH_TEST);

    GL_CHECK(glUseProgram(shaderProgram));

    // Определяем вершины для прямоугольника, представляющего текст
    float vertices[] = {
        -1.0f + 2.0f * x / UIElement::windowWidth, -1.0f + 2.0f * y / UIElement::windowHeight, 0.0f,
        -1.0f + 2.0f * (x + width) / UIElement::windowWidth, -1.0f + 2.0f * y / UIElement::windowHeight, 0.0f,
        -1.0f + 2.0f * (x + width) / UIElement::windowWidth, -1.0f + 2.0f * (y + height) / UIElement::windowHeight, 0.0f,
        -1.0f + 2.0f * x / UIElement::windowWidth, -1.0f + 2.0f * (y + height) / UIElement::windowHeight, 0.0f
    };

    GL_CHECK(glBindVertexArray(vao));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));

    // Установка цвета для текста через униформ переменную
    GLint colorLocation = glGetUniformLocation(shaderProgram, "elementColor");
    if (colorLocation != -1) {
        GL_CHECK(glUniform3f(colorLocation, color.X(), color.Y(), color.Z()));
    }

    // Рисуем прямоугольник, который представляет текст
    GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));

    GL_CHECK(glBindVertexArray(0));
    glEnable(GL_DEPTH_TEST);
}

bool Label::handleClick(float mouseX, float mouseY) {
    return false;  // Метки не обрабатывают клики
} 