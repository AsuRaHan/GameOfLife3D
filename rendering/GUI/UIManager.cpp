#include "UIManager.h"
#include "UIShaders.h"

UIManager::UIManager() {
    // Компиляция шейдеров
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSource = uiVertexShader.c_str();
    GL_CHECK(glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr));
    GL_CHECK(glCompileShader(vertexShader));

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSource = uiFragmentShader.c_str();
    GL_CHECK(glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr));
    GL_CHECK(glCompileShader(fragmentShader));

    uiShaderProgram = glCreateProgram();
    GL_CHECK(glAttachShader(uiShaderProgram, vertexShader));
    GL_CHECK(glAttachShader(uiShaderProgram, fragmentShader));
    GL_CHECK(glLinkProgram(uiShaderProgram));

    GL_CHECK(glDeleteShader(vertexShader));
    GL_CHECK(glDeleteShader(fragmentShader));
}

void UIManager::addElement(std::unique_ptr<UIElement> element) {
    elements.push_back(std::move(element));
}

void UIManager::draw() {
    GL_CHECK(glDisable(GL_DEPTH_TEST));
    for (const auto& element : elements) {
        element->draw(uiShaderProgram);
    }
    GL_CHECK(glEnable(GL_DEPTH_TEST));
}

void UIManager::handleClick(float mouseX, float mouseY) {
    for (const auto& element : elements) {
        if (element->handleClick(mouseX, mouseY)) {
            break;  // Прекращаем обработку после первого успешного клика
        }
    }
}

void UIManager::updateLayout(int windowWidth, int windowHeight) {
    // Здесь можно добавить логику обновления позиций элементов
    // при изменении размера окна
} 