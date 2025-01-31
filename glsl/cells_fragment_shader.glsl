#version 330 core
flat in float vInstanceState;
out vec4 FragColor;

void main()
{
    vec3 color = vInstanceState > 0.5 ? vec3(0.0, 0.6, 0.0) : vec3(0.1, 0.1, 0.1);
    FragColor = vec4(color, 1.0); // Убедитесь, что альфа-канал установлен на 1.0 для полной непрозрачности
}