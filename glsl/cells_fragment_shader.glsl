#version 330 core
flat in float vInstanceState;
in vec3 vInstanceColor;
out vec4 FragColor;

void main()
{
    vec3 color = vInstanceState > 0.5 ? vInstanceColor : vec3(0.1, 0.1, 0.1); // Использование переданного цвета для живых клеток
    FragColor = vec4(color, 1.0);
}