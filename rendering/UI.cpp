#include <windows.h>
#include "UI.h"

int UI::windowWidth = 0;
int UI::windowHeight = 0;
GLuint UI::vao = 0;
GLuint UI::vbo = 0;

// Button implementation
Button::Button(float x, float y, float width, float height, const std::string& text, const Vector3d& color)
    : x(x), y(y), width(width), height(height), text(text), color(color) {
}

void Button::draw() {
    glDisable(GL_DEPTH_TEST);

    GL_CHECK(glUseProgram(UI::getInstance().shaderProgram));

    // ������������� ������� ����
    GLint windowSizeLocation = glGetUniformLocation(UI::getInstance().shaderProgram, "windowSize");
    if (windowSizeLocation != -1) {
        GL_CHECK(glUniform2f(windowSizeLocation, static_cast<float>(UI::windowWidth), static_cast<float>(UI::windowHeight)));
    }

    GL_CHECK(glBindVertexArray(UI::vao));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, UI::vbo));

    // ������ ������ ��� �������������
    float buttonVertices[] = {
        -1.0f + 2.0f * x / UI::windowWidth, -1.0f + 2.0f * y / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * (x + width) / UI::windowWidth, -1.0f + 2.0f * y / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * (x + width) / UI::windowWidth, -1.0f + 2.0f * (y + height) / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * x / UI::windowWidth, -1.0f + 2.0f * (y + height) / UI::windowHeight, 0.0f
    };

    // ������������� ���� ������
    GLint colorLocation = glGetUniformLocation(UI::getInstance().shaderProgram, "elementColor");
    if (colorLocation != -1) {
        GL_CHECK(glUniform3f(colorLocation, color.X(), color.Y(), color.Z()));
    }

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_DYNAMIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));

    // ������ ������
    GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));

    // �������� ���������� ����
    POINT mousePos;
    GetCursorPos(&mousePos);

    // �������� ��������� ����
    HWND hwnd = GetActiveWindow();
    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);

    // ����������� ���������� ���� � ���������� ����
    mousePos.x -= windowRect.left;
    mousePos.y -= windowRect.top;

    // ���������, ��������� �� ���� ��� �������
    bool isMouseOver = (mousePos.x >= x && mousePos.x <= x + width && mousePos.y >= y && mousePos.y <= y + height);

    // ���������, ������ �� ����� ������ ����
    bool isMousePressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (isMousePressed) {
        // ���� ������ ������, ���������, ���� �� ������ �� ������
        if (isMouseOver) {
            // ���� ��, ������� ���������
            MessageBox(hwnd, L"Button clicked!", L"Button", MB_OK);
        }
    }

    // ������������� ���� �������
    Vector3d borderColor;
    if (isMouseOver) {
        borderColor = isMousePressed ? Vector3d(0.0f, 1.0f, 0.0f) : Vector3d(1.0f, 0.0f, 0.0f); // ������� ��� �������, ������� ��� ���������
    }
    else {
        borderColor = Vector3d(1.0f, 1.0f, 1.0f); // ����� � ��������� ������
    }

    GLint borderColorLocation = glGetUniformLocation(UI::getInstance().shaderProgram, "elementColor");
    if (borderColorLocation != -1) {
        GL_CHECK(glUniform3f(borderColorLocation, borderColor.X(), borderColor.Y(), borderColor.Z()));
    }

    // ���������� ��� ����� �������
    float borderVertices[] = {
        // ������� �����
        -1.0f + 2.0f * x / UI::windowWidth, -1.0f + 2.0f * y / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * (x + width) / UI::windowWidth, -1.0f + 2.0f * y / UI::windowHeight, 0.0f,
        // ������ �����
        -1.0f + 2.0f * (x + width) / UI::windowWidth, -1.0f + 2.0f * y / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * (x + width) / UI::windowWidth, -1.0f + 2.0f * (y + height) / UI::windowHeight, 0.0f,
        // ������ �����
        -1.0f + 2.0f * (x + width) / UI::windowWidth, -1.0f + 2.0f * (y + height) / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * x / UI::windowWidth, -1.0f + 2.0f * (y + height) / UI::windowHeight, 0.0f,
        // ����� �����
        -1.0f + 2.0f * x / UI::windowWidth, -1.0f + 2.0f * (y + height) / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * x / UI::windowWidth, -1.0f + 2.0f * y / UI::windowHeight, 0.0f
    };

    // ��������� ����� ������ ��� ����� �������
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_DYNAMIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));

    // ������ ������
    GL_CHECK(glDrawArrays(GL_LINES, 0, 8));

    GL_CHECK(glBindVertexArray(0));
}



void Button::update() {
    // ���������� ��������� ������, ���� ��� ����������
}

void Button::OnWindowResize(int newWidth, int newHeight) {
    // ������� ������ ������������ ������ �� ����������� � ����������� � ����� ������
    x = static_cast<float>(newWidth) / 2.0f - width / 2.0f;
    y = static_cast<float>(newHeight) - height - 10.0f; // 10 �������� �� ������� ����
}

bool Button::isClicked(int mouseX, int mouseY) const {
    return (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height);
}

// TextLabel implementation
TextLabel::TextLabel(float x, float y, const std::string& text, const Vector3d& color)
    : x(x), y(y), text(text), color(color) {
}

void TextLabel::draw() {
    float vertices[] = {
        -1.0f + 2.0f * x / UI::windowWidth, -1.0f + 2.0f * y / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * (x + 50) / UI::windowWidth, -1.0f + 2.0f * y / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * (x + 50) / UI::windowWidth, -1.0f + 2.0f * (y + 20) / UI::windowHeight, 0.0f,
        -1.0f + 2.0f * x / UI::windowWidth, -1.0f + 2.0f * (y + 20) / UI::windowHeight, 0.0f
    };

    GL_CHECK(glBindVertexArray(UI::vao));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, UI::vbo));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));

    // ��������� ����� ��� ������ ����� UI
    UI::getInstance().setElementColor(color);

    GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
    GL_CHECK(glBindVertexArray(0));
}

void TextLabel::update() {
    // ���������� ��������� ������, ���� ��� ����������
}

void TextLabel::OnWindowResize(int newWidth, int newHeight) {
    // ������� ������ ����������� ����� �� ������ ���� � ������� �����
    x = static_cast<float>(newWidth) / 2.0f - 50.0f / 2.0f; // ������������, ��� ����� �������� 50 ������ ������
    y = static_cast<float>(newHeight) - 30.0f; // ������������, ��� ����� �������� 20 ������ ������ � 10 �������� �� �����
}

// UI implementation
UI::UI() {
    loadShaders();
    prepareVAO();
}

UI::~UI() {
    for (auto element : elements) {
        //delete element;
    }
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void UI::addElement(UIElement* element) {
    elements.push_back(element);
}

void UI::draw() {
    glDisable(GL_DEPTH_TEST);

    GL_CHECK(glUseProgram(shaderProgram));
    GL_CHECK(glBindVertexArray(vao));

    // ����� ���� UI ��������
    for (auto element : elements) {
        element->draw();
    }

    GL_CHECK(glBindVertexArray(0));
    glEnable(GL_DEPTH_TEST);
}

void UI::update() {
    for (auto element : elements) {
        element->update();
    }
}

void UI::loadShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        void main()
        {
            gl_Position = vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 elementColor;
        void main()
        {
            FragColor = vec4(elementColor, 1.0);
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    GL_CHECK(glShaderSource(vertexShader, 1, &vertexShaderSource, NULL));
    GL_CHECK(glCompileShader(vertexShader));
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "������ ���������� ���������� �������: " << infoLog << std::endl;
    }

    GL_CHECK(glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL));
    GL_CHECK(glCompileShader(fragmentShader));
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "������ ���������� ������������ �������: " << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    GL_CHECK(glAttachShader(shaderProgram, vertexShader));
    GL_CHECK(glAttachShader(shaderProgram, fragmentShader));
    GL_CHECK(glLinkProgram(shaderProgram));

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "������ �������� ��������� ���������: " << infoLog << std::endl;
    }

    GL_CHECK(glDeleteShader(vertexShader));
    GL_CHECK(glDeleteShader(fragmentShader));

    GL_CHECK(glUseProgram(shaderProgram)); // ���������, ��� ����� ��� ������
}

void UI::prepareVAO() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    if (vao == 0 || vbo == 0) {
        std::cout << "VAO ��� VBO �� �������!" << std::endl;
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    glBindVertexArray(0); // Unbind VAO
}

void UI::OnWindowResize(int newWidth, int newHeight) {
    windowWidth = newWidth;
    windowHeight = newHeight;

    for (auto element : elements) {
        //element->OnWindowResize(newWidth, newHeight);
    }
}

void UI::setElementColor(const Vector3d& color) {
    setUniformColor(color);
}

void UI::setUniformColor(const Vector3d& color) {
    GLint colorLocation = glGetUniformLocation(shaderProgram, "elementColor");
    if (colorLocation != -1) {
        GL_CHECK(glUniform3f(colorLocation, color.X(), color.Y(), color.Z()));
    }
}

// ����� ����� ��� ��������� ���������� UI, �����������, ��� UI - ��� ��������
UI& UI::getInstance() {
    static UI instance; // ������� ������������ ��������� ������ UI
    return instance;
}
