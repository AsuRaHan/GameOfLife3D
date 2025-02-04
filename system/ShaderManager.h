#pragma once
#ifndef SHADERMANAGER_H_
#define SHADERMANAGER_H_

#include "GLFunctions.h"
#include <string>
#include <unordered_map>
#include <fstream>

class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();

    void loadVertexShader(const std::string& name, const char* source);
    void loadFragmentShader(const std::string& name, const char* source);
    void loadComputeShader(const std::string& name, const char* source);
    void linkProgram(const std::string& programName, const std::string& vertexShaderName, const std::string& fragmentShaderName);
    void linkComputeProgram(const std::string& programName, const std::string& computeShaderName);
    void useProgram(const std::string& programName);
    GLuint getProgram(const std::string& programName) const;
    std::string LoadShaderSource(const std::string& filename);
private:
    std::unordered_map<std::string, GLuint> shaders;
    std::unordered_map<std::string, GLuint> programs;

    void compileShader(const char* source, GLenum type, GLuint& shader);
    void checkShaderCompilation(GLuint shader, const std::string& name);
    void checkProgramLinking(GLuint program);
};

#endif // SHADERMANAGER_H_