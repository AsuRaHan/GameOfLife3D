#include "EcosystemAutomaton.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

EcosystemAutomaton::EcosystemAutomaton(int width, int height)
    : GPUAutomaton(width, height) {
    // Вызываем новый шейдер с правилами экосистемы
    CreateComputeShader();

    setUseAdvancedRules(true); // Включаем расширенный режим по умолчанию

    // Начальные правила (будут перезаписаны через UI, но задаём базовые)
    bool birthRules[9] = { 0, 0, 0, 1, 1, 1, 1, 1, 1 };    // B: 1-3
    bool surviveRules[9] = { 0, 0, 1, 1, 1, 1, 1, 1, 1 };  // S: 1-3
    bool overpopRules[9] = { 0, 0, 0, 0, 1, 1, 1, 1, 1 };  // O: 4+
    setBirthRules(birthRules);
    setSurviveRules(surviveRules);
    setOverpopulationRules(overpopRules);
}

void EcosystemAutomaton::CreateComputeShader() {
    CheckComputeLimits(); // Проверяем лимиты, как в базовом классе

    std::string shaderSource = loadShaderSourceWithIncludes("./shader/ecosystemShader.hlsl");
    shaderSource = processShaderMacros(shaderSource);
    
    shaderManager.loadComputeShader("ecosystemShader", shaderSource.c_str());
    shaderManager.linkComputeProgram("ecosystemProgram", "ecosystemShader");
    computeProgram = shaderManager.getProgram("ecosystemProgram");
}

std::string EcosystemAutomaton::loadShaderSourceWithIncludes(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open shader file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    std::string line;
    while (std::getline(file, line)) {
        size_t includePos = line.find("#include");
        if (includePos != std::string::npos) {
            size_t startQuote = line.find('\"');
            size_t endQuote = line.find('\"', startQuote + 1);
            if (startQuote != std::string::npos && endQuote != std::string::npos) {
                std::string includeFilename = line.substr(startQuote + 1, endQuote - startQuote - 1);
                std::string includePath = "./shader/" + includeFilename; // Assuming includes are in the "shader" directory
                buffer << loadShaderSourceWithIncludes(includePath); // Recursive call for nested includes
            }
        } else {
            buffer << line << "\n";
        }
    }
    file.close();
    return buffer.str();
}

std::string EcosystemAutomaton::processShaderMacros(std::string shaderSource) {
    shaderSource = replaceMacros(shaderSource, "{{groupSizeX}}", std::to_string(groupSizeX));
    shaderSource = replaceMacros(shaderSource, "{{groupSizeY}}", std::to_string(groupSizeY));
    return shaderSource;
}

std::string EcosystemAutomaton::replaceMacros(std::string source, const std::string& macro, const std::string& value) {
    size_t pos = 0;
    while ((pos = source.find(macro, pos)) != std::string::npos) {
        source.replace(pos, macro.length(), value);
        pos += value.length();
    }
    return source;
}
