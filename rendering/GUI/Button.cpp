#include "Button.h"

Button::Button(float x, float y, float width, float height,
    const std::string& text, const Vector3d& color)
    : UIElement(x, y, width, height, text, color) {
}

//void Button::draw(GLuint shaderProgram, GLuint vao, GLuint vbo) {
//    glDisable(GL_DEPTH_TEST);
//
//    GL_CHECK(glUseProgram(shaderProgram));
//
//    // Определяем вершины кнопки в нормализованных координатах
//    float buttonVertices[] = {
//        -1.0f + 2.0f * x / UIElement::windowWidth, -1.0f + 2.0f * y / UIElement::windowHeight, 0.0f,
//        -1.0f + 2.0f * (x + width) / UIElement::windowWidth, -1.0f + 2.0f * y / UIElement::windowHeight, 0.0f,
//        -1.0f + 2.0f * (x + width) / UIElement::windowWidth, -1.0f + 2.0f * (y + height) / UIElement::windowHeight, 0.0f,
//        -1.0f + 2.0f * x / UIElement::windowWidth, -1.0f + 2.0f * (y + height) / UIElement::windowHeight, 0.0f
//    };
//
//    GL_CHECK(glBindVertexArray(vao));
//    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
//    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_DYNAMIC_DRAW));
//    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
//    GL_CHECK(glEnableVertexAttribArray(0));
//
//    // Установка цвета кнопки
//    GLint colorLocation = glGetUniformLocation(shaderProgram, "elementColor");
//    if (colorLocation != -1) {
//        GL_CHECK(glUniform3f(colorLocation, color.X(), color.Y(), color.Z()));
//    }
//
//    // Рисуем кнопку
//    GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
//
//
//    // Устанавливаем цвет бордюра
//    Vector3d borderColor;
//    if (isHovered) {
//        borderColor =  Vector3d(0.0f, 1.0f, 0.0f); // Зеленый при нажатии, красный при наведении
//    }
//    else {
//        borderColor = Vector3d(1.0f, 1.0f, 1.0f); // Белый в противном случае
//    }
//
//    if (colorLocation != -1) {
//        GL_CHECK(glUniform3f(colorLocation, borderColor.X(), borderColor.Y(), borderColor.Z()));
//    }
//
//    // Координаты для линий бордюра
//    float borderVertices[] = {
//        // Верхняя линия
//        -1.0f + 2.0f * x / UIElement::windowWidth, -1.0f + 2.0f * y / UIElement::windowHeight, 0.0f,
//        -1.0f + 2.0f * (x + width) / UIElement::windowWidth, -1.0f + 2.0f * y / UIElement::windowHeight, 0.0f,
//        // Правая линия
//        -1.0f + 2.0f * (x + width) / UIElement::windowWidth, -1.0f + 2.0f * y / UIElement::windowHeight, 0.0f,
//        -1.0f + 2.0f * (x + width) / UIElement::windowWidth, -1.0f + 2.0f * (y + height) / UIElement::windowHeight, 0.0f,
//        // Нижняя линия
//        -1.0f + 2.0f * (x + width) / UIElement::windowWidth, -1.0f + 2.0f * (y + height) / UIElement::windowHeight, 0.0f,
//        -1.0f + 2.0f * x / UIElement::windowWidth, -1.0f + 2.0f * (y + height) / UIElement::windowHeight, 0.0f,
//        // Левая линия
//        -1.0f + 2.0f * x / UIElement::windowWidth, -1.0f + 2.0f * (y + height) / UIElement::windowHeight, 0.0f,
//        -1.0f + 2.0f * x / UIElement::windowWidth, -1.0f + 2.0f * y / UIElement::windowHeight, 0.0f
//    };
//
//    // Обновляем буфер вершин для линий бордюра
//    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_DYNAMIC_DRAW));
//    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
//    GL_CHECK(glEnableVertexAttribArray(0));
//
//    // Рисуем бордюр
//    GL_CHECK(glDrawArrays(GL_LINES, 0, 8));
//
//    GL_CHECK(glBindVertexArray(0));
//    glEnable(GL_DEPTH_TEST);
//}
void Button::draw(GLuint shaderProgram, GLuint vao, GLuint vbo) {
    glDisable(GL_DEPTH_TEST);

    GL_CHECK(glUseProgram(shaderProgram));
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after glUseProgram: " << error << std::endl;
    }
    // Преобразование координат в нормализованные координаты OpenGL
    float x1 = -1.0f + 2.0f * x / UIElement::windowWidth;
    float y1 = -1.0f + 2.0f * y / UIElement::windowHeight;
    float x2 = -1.0f + 2.0f * (x + width) / UIElement::windowWidth;
    float y2 = -1.0f + 2.0f * (y + height) / UIElement::windowHeight;

    // Тело кнопки
    float bodyVertices[] = {
        x1, y1, 0.0f,
        x2, y1, 0.0f,
        x2, y2, 0.0f,
        x1, y2, 0.0f
    };

    // Бордюр кнопки
    float borderVertices[] = {
        // Верхняя линия
        x1, y1, 0.0f, x2, y1, 0.0f,
        // Правая линия
        x2, y1, 0.0f, x2, y2, 0.0f,
        // Нижняя линия
        x2, y2, 0.0f, x1, y2, 0.0f,
        // Левая линия
        x1, y2, 0.0f, x1, y1, 0.0f
    };
    std::cout << "Button at " << x << ", " << y << ", size: " << width << "x" << height << std::endl;
    GL_CHECK(glBindVertexArray(vao));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after binding VAO/VBO: " << error << std::endl;
    }

    // Отрисовка тела кнопки
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(bodyVertices), bodyVertices, GL_DYNAMIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));

    // Установка цвета кнопки
    GLint colorLocation = glGetUniformLocation(shaderProgram, "elementColor");
    if (colorLocation == -1) {
        std::cerr << "Failed to find uniform location for 'elementColor'" << std::endl;
    }

    if (colorLocation != -1) {
        GL_CHECK(glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f)); // Красный цвет для теста
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error after glUniform3f: " << error << std::endl;
        }
    }
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << error << std::endl;
    }
    //GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
    GL_CHECK(glDrawArrays(GL_LINE_LOOP, 0, 4)); // Для теста можно использовать GL_LINE_LOOP

    // Отрисовка бордюра
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_DYNAMIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));

    // Установка цвета бордюра с учетом эффекта наведения
    Vector3d borderColor;
    if (isHovered) {
        borderColor = Vector3d(1.0f, 0.0f, 0.0f); // Красный при наведении
    }
    else {
        borderColor = Vector3d(0.0f, 0.0f, 0.0f); // Чёрный при отсутствии наведения
    }
    if (colorLocation != -1) {
        GL_CHECK(glUniform3f(colorLocation, borderColor.X(), borderColor.Y(), borderColor.Z()));
    }
    GL_CHECK(glDrawArrays(GL_LINES, 0, 8));

    GL_CHECK(glBindVertexArray(0));
    glEnable(GL_DEPTH_TEST);
}


bool Button::isClicked(int mouseX, int mouseY) const {
    return (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height);
}

//void Button::OnWindowResize(int newWidth, int newHeight) {
//
//
//}

bool Button::handleClick(float mouseX, float mouseY) {
    if (!isVisible) return false;

    bool wasClicked = UIElement::handleClick(mouseX, mouseY);
    if (wasClicked && onClick) {
        onClick();
    }
    return wasClicked;
}

void Button::handleMouseMove(float mouseX, float mouseY) {
    bool wasHovered = isHovered;
    isHovered = (mouseX >= x && mouseX <= x + width &&
        mouseY >= y && mouseY <= y + height);

    // Обновление цвета при наведении
    if (isHovered && !wasHovered) {
        hoverColor = Vector3d(
            (color.X() * 1.2f > 1.0f) ? 1.0f : color.X() * 1.2f,
            (color.Y() * 1.2f > 1.0f) ? 1.0f : color.Y() * 1.2f,
            (color.Z() * 1.2f > 1.0f) ? 1.0f : color.Z() * 1.2f
        );
    }
}