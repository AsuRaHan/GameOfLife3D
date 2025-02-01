#version 330 core
layout(location = 0) in vec2 aPos; 
layout(location = 1) in vec2 aInstancePos;
layout(location = 2) in float aInstanceState; 
layout(location = 3) in vec3 aInstanceColor; // Убедитесь, что это значение соответствует вашему расположению атрибутов

uniform mat4 projection;
uniform mat4 view;
uniform float cellSize; 

flat out float vInstanceState;
out vec3 vInstanceColor;

void main()
{
    gl_Position = projection * view * vec4(aPos * cellSize + aInstancePos, 0.0, 1.0);
    vInstanceState = aInstanceState;
    vInstanceColor = aInstanceColor; // Передача цвета в фрагментный шейдер
}