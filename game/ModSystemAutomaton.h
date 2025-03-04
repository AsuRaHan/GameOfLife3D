#pragma once
#ifndef MODSYSTEM_AUTOMATON_H
#define MODSYSTEM_AUTOMATON_H

#include "GPUAutomaton.h"
#include "ModManager.h"
#include "UIElement.h"

#include <string>
#include <unordered_set> // Для хранения уникальных имен файлов
#include <vector>
#include <map> // Добавляем map
#include <functional> // Для std::function

//struct ShaderVariableInfo {
//    std::string name;
//    GLint location;
//    GLenum type;
//    GLint arraySize;
//    std::function<void(const ShaderVariableInfo&, std::string)> callback; // Callback function
//    // Убираем glm::vec4 и glm::ivec2
//    union {
//        int i;
//        float f;
//        bool b;
//    } value;
//    std::string valueStr = ""; // value in str
//    // Add max min value
//    float min = 0.0f;
//    float max = 0.0f;
//    
//    // Добавляем массив для хранения значений, если это массив
//    std::vector<float> values; 
//};

class ModSystemAutomaton : public GPUAutomaton {
public:
    ModSystemAutomaton(int width, int height);
    ~ModSystemAutomaton() override = default;
    void LoadSelectedMod();

    void getModShaderVariables();
    void updateShaderUniforms();
    void addUIElementsToVariablesMap();

    void beforeUpdate() override;
protected:
    void CreateComputeShader() override; // Переопределяем шейдер для новых правил
private:
    std::map<std::string, ShaderVariableInfo*> variablesMap; // Мапа для быстрого доступа к переменным
    std::unordered_set<std::string> includedFiles; // Список уже подключенных файлов
    std::string loadShaderSourceWithIncludes(const std::string& filename);
    std::string processShaderMacros(std::string shaderSource);
    std::string replaceMacros(std::string source, const std::string& macro, const std::string& value);
};

#endif