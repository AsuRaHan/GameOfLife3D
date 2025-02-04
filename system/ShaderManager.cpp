#include "ShaderManager.h"
#include <iostream>

ShaderManager::ShaderManager() {}

ShaderManager::~ShaderManager() {
    for (auto& pair : shaders) {
        glDeleteShader(pair.second);
    }
    for (auto& pair : programs) {
        glDeleteProgram(pair.second);
    }
}

void ShaderManager::loadVertexShader(const std::string& name, const char* source) {
    GLuint shader;
    compileShader(source, GL_VERTEX_SHADER, shader);
    shaders[name] = shader;
}

void ShaderManager::loadFragmentShader(const std::string& name, const char* source) {
    GLuint shader;
    compileShader(source, GL_FRAGMENT_SHADER, shader);
    shaders[name] = shader;
}

void ShaderManager::loadComputeShader(const std::string& name, const char* source) {
    GLuint shader;
    compileShader(source, GL_COMPUTE_SHADER, shader);
    shaders[name] = shader;
}

void ShaderManager::linkProgram(const std::string& programName, const std::string& vertexShaderName, const std::string& fragmentShaderName) {
    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[vertexShaderName]);
    glAttachShader(program, shaders[fragmentShaderName]);
    glLinkProgram(program);
    checkProgramLinking(program);
    programs[programName] = program;
}

void ShaderManager::linkComputeProgram(const std::string& programName, const std::string& computeShaderName) {
    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[computeShaderName]);
    glLinkProgram(program);
    checkProgramLinking(program);
    programs[programName] = program;
}

void ShaderManager::useProgram(const std::string& programName) {
    glUseProgram(programs[programName]);
}

GLuint ShaderManager::getProgram(const std::string& programName) const {
    return programs.at(programName);
}

void ShaderManager::compileShader(const char* source, GLenum type, GLuint& shader) {
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    checkShaderCompilation(shader, "Shader");
}

void ShaderManager::checkShaderCompilation(GLuint shader, const std::string& name) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Ошибка компиляции " << name << ": " << infoLog << std::endl;
    }
}

void ShaderManager::checkProgramLinking(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Ошибка линковки программы: " << infoLog << std::endl;
    }
}
std::string ShaderManager::LoadShaderSource(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        //throw std::runtime_error("Failed to open shader file: " + filename);
        std::cout << "Не удалось загрузить файл шейдерной программы: " << filename << std::endl;
        exit(1);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}