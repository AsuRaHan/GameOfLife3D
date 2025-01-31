#version 330 core
out vec4 FragColor;

uniform vec4 overlayColor; // Добавляем uniform переменную для цвета

void main()
{
    FragColor = overlayColor; // Используем переданный цвет
}