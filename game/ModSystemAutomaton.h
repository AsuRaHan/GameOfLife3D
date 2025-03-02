#pragma once
#ifndef MODSYSTEM_AUTOMATON_H
#define MODSYSTEM_AUTOMATON_H

#include "GPUAutomaton.h"
#include "ModManager.h"

#include <string>
#include <unordered_set> // Для хранения уникальных имен файлов
#include <vector>

class ModSystemAutomaton : public GPUAutomaton {
public:
    ModSystemAutomaton(int width, int height);
    ~ModSystemAutomaton() override = default;
    void LoadSelectedMod();
protected:
    void CreateComputeShader() override; // Переопределяем шейдер для новых правил
private:

    std::unordered_set<std::string> includedFiles; // Список уже подключенных файлов
    std::string loadShaderSourceWithIncludes(const std::string& filename);
    std::string processShaderMacros(std::string shaderSource);
    std::string replaceMacros(std::string source, const std::string& macro, const std::string& value);
};

#endif