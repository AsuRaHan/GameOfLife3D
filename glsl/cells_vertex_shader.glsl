#version 330 core
layout(location = 0) in vec2 aPos; 
layout(location = 1) in vec2 aInstancePos;
layout(location = 2) in float aInstanceState; 

uniform mat4 projection;
uniform mat4 view;
uniform float cellSize; // Добавляем объявление uniform переменной для cellSize

flat out float vInstanceState;

void main()
{
    // Используем позицию инстанса для смещения
    gl_Position = projection * view * vec4(aPos * cellSize + aInstancePos, 0.0, 1.0);
    vInstanceState = aInstanceState;
}