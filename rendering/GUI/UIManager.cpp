#include "UIManager.h"
#include "UIShaders.h"
#include "Button.h"

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
    float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
    float baseScale = (windowWidth < windowHeight) ? (windowWidth / 1000.0f) : (windowHeight / 1000.0f); // Базовый масштаб

    for (auto& element : elements) {
        // Получаем текущие нормализованные координаты
        float normalizedX = element->getX();
        float normalizedY = element->getY();
        float normalizedWidth = element->getWidth();
        float normalizedHeight = element->getHeight();

        // Масштабируем размеры с учетом размера окна
        float scaledWidth = normalizedWidth * baseScale;
        float scaledHeight = normalizedHeight * baseScale;

        // Корректируем позиции с учетом aspect ratio
        float scaledX = normalizedX;
        float scaledY = normalizedY;

        // Если окно шире стандартного соотношения (16:9)
        if (aspectRatio > 16.0f / 9.0f) {
            // Корректируем по горизонтали
            scaledX = normalizedX * (aspectRatio / (16.0f / 9.0f));
            scaledWidth *= (aspectRatio / (16.0f / 9.0f));
        }
        // Если окно выше стандартного соотношения
        else if (aspectRatio < 16.0f / 9.0f) {
            // Корректируем по вертикали
            scaledY = normalizedY * ((9.0f / 16.0f) / aspectRatio);
            scaledHeight *= ((9.0f / 16.0f) / aspectRatio);
        }

        // Применяем минимальные размеры
        float minSize = 0.05f; // Минимальный размер элемента
        scaledWidth = (scaledWidth < minSize) ? minSize : scaledWidth;
        scaledHeight = (scaledHeight < minSize) ? minSize : scaledHeight;

        // Обновляем позиции и размеры элементов
        element->setPosition(scaledX, scaledY);
        element->setSize(scaledWidth, scaledHeight);
    }
}


void UIManager::handleMouseMove(float x, float y) {
    for (auto& element : elements) {
        if (auto button = dynamic_cast<Button*>(element.get())) {
            button->handleMouseMove(x, y);
        }
    }
} 