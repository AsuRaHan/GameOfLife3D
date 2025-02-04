#include "UIElement.h"

// Инициализация статических членов
GLuint UIElement::vao = 0;
GLuint UIElement::vbo = 0;
int UIElement::windowWidth = 0;
int UIElement::windowHeight = 0;

UIElement::UIElement(float x, float y, float width, float height,
    const std::string& text, const Vector3d& color)
    : x(x), y(y), width(width), height(height),
    isVisible(true), color(color), text(text) {
}

void UIElement::screenToNormalized(int screenX, int screenY, int windowWidth, int windowHeight,
    float& normalizedX, float& normalizedY) {
    normalizedX = (2.0f * screenX) / windowWidth - 1.0f;
    normalizedY = 1.0f - (2.0f * screenY) / windowHeight;
}

bool UIElement::handleClick(float mouseX, float mouseY) {
    return mouseX >= x - width / 2 && mouseX <= x + width / 2 &&
        mouseY >= y - height / 2 && mouseY <= y + height / 2;
}

void UIElement::OnWindowResize(int newWidth, int newHeight) {
    windowWidth = newWidth;
    windowHeight = newHeight;

}