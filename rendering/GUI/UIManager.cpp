#include "UIManager.h"
#include "Button.h"

UIManager::UIManager() {
    loadShaders();
    prepareVAO();
}

void UIManager::prepareVAO() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void UIManager::loadShaders() {
    // Загружаем шейдеры
    shaderManager.loadVertexShader("uiVertexShader", R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        void main()
        {
            gl_Position = vec4(aPos, 1.0);
        }
    )");

    shaderManager.loadFragmentShader("uiFragmentShader", R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 elementColor;
        void main()
        {
            FragColor = vec4(elementColor, 1.0);
        }
    )");

    // Линкуем программу
    shaderManager.linkProgram("uiShaderProgram", "uiVertexShader", "uiFragmentShader");
    shaderProgram = shaderManager.getProgram("uiShaderProgram"); // Присваиваем shaderProgram ID программы
}

void UIManager::addElement(std::unique_ptr<UIElement> element) {
    elements.push_back(std::move(element));
}

//void UIManager::draw() {
//    glDisable(GL_DEPTH_TEST);
//    GL_CHECK(glUseProgram(shaderProgram));
//    GL_CHECK(glBindVertexArray(vao));
//
//    for (const auto& element : elements) {
//        element->draw(shaderProgram, vao, vbo);
//    }
//
//    GL_CHECK(glBindVertexArray(0));
//    glEnable(GL_DEPTH_TEST);
//}
void UIManager::draw() {
    glDisable(GL_DEPTH_TEST);
    GL_CHECK(glUseProgram(shaderProgram));
    GL_CHECK(glBindVertexArray(vao));

    std::cout << "Drawing " << elements.size() << " elements." << std::endl;
    for (const auto& element : elements) {
        std::cout << "Drawing element at " << element->getX() << ", " << element->getY() << std::endl;
        element->draw(shaderProgram, vao, vbo);
    }

    GL_CHECK(glBindVertexArray(0));
    glEnable(GL_DEPTH_TEST);
}
void UIManager::setElementColor(const Vector3d& color) {
    GLint colorLoc = glGetUniformLocation(shaderProgram, "elementColor");
    if (colorLoc != -1) {
        GL_CHECK(glUniform3f(colorLoc, color.X(), color.Y(), color.Z()));
    }
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
    float baseScale = (windowWidth < windowHeight) ? static_cast<float>(windowWidth) / 1000.0f : static_cast<float>(windowHeight) / 1000.0f;

    for (auto& element : elements) {
        element->OnWindowResize(windowWidth, windowHeight);

        float normalizedX = element->getX();
        float normalizedY = element->getY();
        float normalizedWidth = element->getWidth();
        float normalizedHeight = element->getHeight();

        float scaleX = baseScale;
        float scaleY = baseScale;

        if (aspectRatio > 1.0f) { // Широкоформатный экран
            scaleY /= aspectRatio;
        }
        else {
            scaleX *= aspectRatio;
        }

        float scaledWidth = normalizedWidth * scaleX;
        float scaledHeight = normalizedHeight * scaleY;

        // Нормализация координат
        float aspectScaleX = windowWidth < 800 ? static_cast<float>(windowWidth) / 800.0f : 1.0f;
        float aspectScaleY = windowHeight < 600 ? static_cast<float>(windowHeight) / 600.0f : 1.0f;

        float scaledX = (normalizedX * 2.0f - 1.0f) * aspectScaleX;
        float scaledY = (normalizedY * 2.0f - 1.0f) * aspectScaleY;

        // Ограничение масштабирования, чтобы элементы не уходили за пределы экрана
        if (scaledX > 1.0f) scaledX = 1.0f;
        if (scaledX < -1.0f) scaledX = -1.0f;
        if (scaledY > 1.0f) scaledY = 1.0f;
        if (scaledY < -1.0f) scaledY = -1.0f;

        //element->setPosition(scaledX, scaledY);
        //element->setSize(scaledWidth, scaledHeight);
    }
}


void UIManager::handleMouseMove(float x, float y) {
    for (auto& element : elements) {
        if (auto button = dynamic_cast<Button*>(element.get())) {
            button->handleMouseMove(x, y);
        }
    }
} 