#include "ShaderManager.h"


ShaderManager::ShaderManager() {}

ShaderManager::~ShaderManager() {
    for (auto& pair : shaders) {
        if (pair.second != 0) { // Проверяем, что шейдер существует
            GL_CHECK(glDeleteShader(pair.second));
        }
    }
    for (auto& pair : programs) {
        if (pair.second != 0) { // Проверяем, что программа существует
            GL_CHECK(glDeleteProgram(pair.second));
        }
    }
}

void ShaderManager::loadVertexShader(const std::string& name, const char* source) {
    std::cout << "Начинаю загрузку вершинного шейдера " << name << std::endl;
    GLuint shader;
    compileShader(source, GL_VERTEX_SHADER, shader);
    shaders[name] = shader;
}

void ShaderManager::loadFragmentShader(const std::string& name, const char* source) {
    std::cout << "Начинаю загрузку фрагментного шейдера " << name << std::endl;
    GLuint shader;
    compileShader(source, GL_FRAGMENT_SHADER, shader);
    shaders[name] = shader;
}

void ShaderManager::loadGeometryShader(const std::string& name, const char* source) {
    std::cout << "Начинаю загрузку геометрического шейдера " << name << std::endl;
    GLuint shader;
    compileShader(source, GL_GEOMETRY_SHADER, shader);
    shaders[name] = shader;
}

void ShaderManager::loadTessControlShader(const std::string& name, const char* source) {
    std::cout << "Начинаю загрузку контрольного шейдера тесселяции " << name << std::endl;
    GLuint shader;
    compileShader(source, GL_TESS_CONTROL_SHADER, shader);
    shaders[name] = shader;
}

void ShaderManager::loadTessEvaluationShader(const std::string& name, const char* source) {
    std::cout << "Начинаю загрузку оценочного шейдера тесселяции " << name << std::endl;
    GLuint shader;
    compileShader(source, GL_TESS_EVALUATION_SHADER, shader);
    shaders[name] = shader;
}

void ShaderManager::loadComputeShader(const std::string& name, const char* source) {
    std::cout << "Начинаю загрузку вычислительного шейдера " << name << std::endl;
    // Удаляем старый шейдер, если он есть
    if (shaders.find(name) != shaders.end()) {
        GL_CHECK(glDeleteShader(shaders[name]));
        shaders.erase(name);
    }

    GLuint shader;
    compileShader(source, GL_COMPUTE_SHADER, shader);
    shaders[name] = shader;
}



void ShaderManager::linkProgram(const std::string& programName, const std::string& vertexShaderName, const std::string& fragmentShaderName) {
    std::cout << "[ShaderManager::linkProgram] Начинаю компиляцию шейдерной программы programName=" << programName 
        << " vertexShaderName=" << vertexShaderName 
        << " fragmentShaderName=" << fragmentShaderName << std::endl;
    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[vertexShaderName]);
    glAttachShader(program, shaders[fragmentShaderName]);
    glLinkProgram(program);
    checkProgramLinking(program);
    programs[programName] = program;
    std::cout << "[ShaderManager::linkProgram] Компиляция шейдерной программы programName=" << programName << " завершена ей присвоен ID=" << program << std::endl;
}

void ShaderManager::linkGeometryProgram(const std::string& programName, const std::string& vertexShaderName, 
    const std::string& geometryShaderName, const std::string& fragmentShaderName) {

    std::cout << "[ShaderManager::linkGeometryProgram] Начинаю компиляцию геометрической шейдерной программы programName=" << programName
        << " vertexShaderName=" << vertexShaderName
        << " geometryShaderName=" << geometryShaderName
        << " fragmentShaderName=" << fragmentShaderName << std::endl;
    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[vertexShaderName]);
    glAttachShader(program, shaders[geometryShaderName]);
    glAttachShader(program, shaders[fragmentShaderName]);
    glLinkProgram(program);
    checkProgramLinking(program);
    programs[programName] = program;
    std::cout << "[ShaderManager::linkGeometryProgram] Компиляция шейдерной программы programName=" << programName << " завершена ей присвоен ID=" << program << std::endl;
}

void ShaderManager::linkTessellationProgram(const std::string& programName, const std::string& vertexShaderName, 
    const std::string& tessControlShaderName, const std::string& tessEvaluationShaderName, const std::string& fragmentShaderName) {

    std::cout << "[ShaderManager::linkTessellationProgram] Начинаю компиляцию шейдерной программы тесселяции programName=" << programName
        << " vertexShaderName=" << vertexShaderName
        << " tessControlShaderName=" << tessControlShaderName
        << " tessEvaluationShaderName=" << tessEvaluationShaderName
        << " fragmentShaderName=" << fragmentShaderName << std::endl;

    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[vertexShaderName]);
    glAttachShader(program, shaders[tessControlShaderName]);
    glAttachShader(program, shaders[tessEvaluationShaderName]);
    glAttachShader(program, shaders[fragmentShaderName]);
    glLinkProgram(program);
    checkProgramLinking(program);
    programs[programName] = program;
    std::cout << "[ShaderManager::linkTessellationProgram] Компиляция шейдерной программы programName=" << programName << " завершена ей присвоен ID=" << program << std::endl;
}


void ShaderManager::linkComputeProgram(const std::string& programName, const std::string& computeShaderName) {
    std::cout << "[ShaderManager::linkComputeProgram] Начинаю компиляцию шейдерной программы programName=" << programName
        << " computeShaderName=" << computeShaderName << std::endl;
    // Удаляем старую программу, если она есть
    if (programs.find(programName) != programs.end()) {
        GL_CHECK(glDeleteProgram(programs[programName]));
        programs.erase(programName);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[computeShaderName]);
    glLinkProgram(program);
    checkProgramLinking(program);
    programs[programName] = program;
    std::cout << "[ShaderManager::linkComputeProgram] Компиляция шейдерной программы programName=" << programName << " завершена ей присвоен ID=" << program << std::endl;
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
        std::cerr << "[ShaderManager::checkShaderCompilation] Ошибка компиляции " << name << ": " << infoLog << std::endl;
    }
}

void ShaderManager::checkProgramLinking(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "[ShaderManager::checkProgramLinking] Ошибка линковки программы: " << infoLog << " ID=" << program << std::endl;
    }
}
std::string ShaderManager::LoadShaderSource(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось загрузить файл шейдерной программы: " + filename);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}