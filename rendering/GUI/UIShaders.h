const std::string uiVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform vec2 uPosition;
uniform vec2 uScale;
out vec2 TexCoord;

void main() {
    vec2 pos = aPos * uScale + uPosition;
    gl_Position = vec4(pos, 0.0, 1.0);
    TexCoord = aPos * 0.5 + 0.5;
}
)";

const std::string uiFragmentShader = R"(
#version 330 core
in vec2 TexCoord;
uniform vec3 uColor;
uniform float uHovered;
out vec4 FragColor;

void main() {
    vec3 color = mix(uColor, uColor * 1.2, uHovered);
    FragColor = vec4(color, 1.0);
}
)"; 