#pragma once
#ifndef MODSYSTEM_AUTOMATON_H
#define MODSYSTEM_AUTOMATON_H

#include "GPUAutomaton.h"

class ModSystemAutomaton : public GPUAutomaton {
public:
    ModSystemAutomaton(int width, int height);
    ~ModSystemAutomaton() override = default;

protected:
    void CreateComputeShader() override; // Переопределяем шейдер для новых правил
private:
    std::string loadShaderSourceWithIncludes(const std::string& filename);
    std::string processShaderMacros(std::string shaderSource);
    std::string replaceMacros(std::string source, const std::string& macro, const std::string& value);
};

#endif