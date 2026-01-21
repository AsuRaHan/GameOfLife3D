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

class ModSystemAutomaton : public GPUAutomaton {
public:
    ModSystemAutomaton(int width, int height);
    ~ModSystemAutomaton() override = default;
    void LoadSelectedMod();

    void getModShaderVariables();

    void updateShaderUniforms();

    void beforeUpdate() override;
protected:
    void CreateComputeShader() override; // Переопределяем шейдер для новых правил
private:
    std::map<std::string, ShaderVariableInfo> variablesMap; // Объявление переменной variablesMap
    std::unordered_set<std::string> includedFiles; // Список уже подключенных файлов
	std::map<std::string, GLint> systemUniforms; // Кэш системных uniform-ов

    std::string loadShaderSourceWithIncludes(const std::string& filename);
    std::string processShaderMacros(std::string shaderSource);
    std::string replaceMacros(std::string source, const std::string& macro, const std::string& value);

    void cacheSystemUniformLocations();
    void updateSystemUniforms();
};

#endif