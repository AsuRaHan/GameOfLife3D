#pragma once
#ifndef ECOSYSTEM_AUTOMATON_H
#define ECOSYSTEM_AUTOMATON_H

#include "GPUAutomaton.h"

class EcosystemAutomaton : public GPUAutomaton {
public:
    EcosystemAutomaton(int width, int height);
    ~EcosystemAutomaton() override = default;

protected:
    void CreateComputeShader() override; // Переопределяем шейдер для новых правил
private:
    std::string loadShaderSourceWithIncludes(const std::string& filename);
    std::string processShaderMacros(std::string shaderSource);
    std::string replaceMacros(std::string source, const std::string& macro, const std::string& value);
};

#endif