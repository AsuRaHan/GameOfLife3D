#include "ModSystemAutomaton.h"
#include <iostream> // Добавляем для std::cerr
#include <sstream> // Добавляем для std::stringstream
#include <imgui.h> //Добавляем для imgui
#include <iomanip> // Добавляем для работы с float в string
#include <algorithm> // Для std::min, std::max


ModSystemAutomaton::ModSystemAutomaton(int width, int height)
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

void ModSystemAutomaton::CreateComputeShader() {
    CheckComputeLimits(); // Проверяем лимиты, как в базовом классе

    //LoadSelectedMod();
}

std::string ModSystemAutomaton::loadShaderSourceWithIncludes(const std::string& filename) {
    // Проверяем, был ли этот файл уже подключен
    if (includedFiles.find(filename) != includedFiles.end()) {
        std::cout << "Skipping already included file: " << filename << std::endl;
        return ""; // Пропускаем повторное подключение
    }

    // Добавляем имя файла в список подключенных
    includedFiles.insert(filename);

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
                std::string includePath = "./mods/libs/" + includeFilename; // Assuming includes are in the "shader" directory
                // Изменено
                if (!std::filesystem::exists(includePath)) {
                    //попробуем найти в папке mods
                    includePath = ModManager::getModFilePath(includeFilename);
                }
                buffer << loadShaderSourceWithIncludes(includePath); // Recursive call for nested includes
            }
        }
        else {
            buffer << line << "\n";
        }
    }
    file.close();
    return buffer.str();
}

std::string ModSystemAutomaton::processShaderMacros(std::string shaderSource) {
    shaderSource = replaceMacros(shaderSource, "{{groupSizeX}}", std::to_string(groupSizeX));
    shaderSource = replaceMacros(shaderSource, "{{groupSizeY}}", std::to_string(groupSizeY));
    return shaderSource;
}

std::string ModSystemAutomaton::replaceMacros(std::string source, const std::string& macro, const std::string& value) {
    size_t pos = 0;
    while ((pos = source.find(macro, pos)) != std::string::npos) {
        source.replace(pos, macro.length(), value);
        pos += value.length();
    }
    return source;
}

void ModSystemAutomaton::LoadSelectedMod() {
    // Очищаем список
    includedFiles.clear();

    // Получаем имя текущего мода из ModManager
    std::string currentModName = ModManager::getCurrentModName();

    // Формируем путь к файлу main.glsl в папке текущего мода
    std::string mainShaderPath = ModManager::getModsFolderPath() + "/" + currentModName + "/main.glsl";

    // Если currentModName пустая, то используем базовый шейдер
    if (currentModName.empty()) {
        currentModName = "modystemshader";
        mainShaderPath = "./mods/modystemshader/main.glsl";
        std::cerr << "Warning: mod name is empty! Loading base shader" << std::endl;
    }

    // Загружаем шейдер
    std::string shaderSource = loadShaderSourceWithIncludes(mainShaderPath);
    shaderSource = processShaderMacros(shaderSource);

    //std::cout << "Shader source before compilation:\n" << shaderSource << std::endl;

    // Формируем имя программы на основе имени мода
    std::string programName = currentModName + "Program";
    std::string shaderName = currentModName + "Shader";

    // Загружаем и линкуем шейдер с динамическим именем
    shaderManager.loadComputeShader(shaderName, shaderSource.c_str());
    shaderManager.linkComputeProgram(programName, shaderName);
    computeProgram = shaderManager.getProgram(programName);

    getModShaderVariables();

}

void ModSystemAutomaton::beforeUpdate() {
    // Здесь ваш код, который должен выполняться перед обновлением
    // Например, обновление uniform-ов из UI
    //updateShaderUniforms();
    //std::cout << "ModSystemAutomaton::beforeUpdate() called" << std::endl;
}

void ModSystemAutomaton::getModShaderVariables() {
    if (computeProgram == 0) {
        std::cerr << "Error: Shader program not found: " << computeProgram << std::endl;
        return;
    }

    variablesMap.clear();
    std::vector<UIElement> uiElements;

    GLint numUniforms;
    glGetProgramiv(computeProgram, GL_ACTIVE_UNIFORMS, &numUniforms);
    std::cout << "Number of active uniforms: " << numUniforms << std::endl;

    for (int i = 0; i < numUniforms; ++i) {
        GLchar name[256];
        GLint size;
        GLenum type;
        glGetActiveUniform(computeProgram, i, 256, NULL, &size, &type, name);
        GLint location = glGetUniformLocation(computeProgram, name);

        std::cout << "Raw type value for " << name << ": " << type << " (hex: 0x" << std::hex << type << ")" << std::dec << std::endl;
        std::cout << "Uniform: " << name << " Type (hex): 0x" << std::hex << type
            << " Type (dec): " << std::dec << (int)type
            << " [DEBUG: GL_INT=" << (int)GL_INT << ", GL_INT_VEC2=" << (int)GL_INT_VEC2 << ", GL_BOOL=" << (int)GL_BOOL << "]"
            << " Location: " << location << " Size: " << size << std::endl;

        std::string varName = std::string(name);
        if (varName.rfind("gl_", 0) == 0) continue;

        ShaderVariableInfo varInfo;
        varInfo.name = varName;
        varInfo.type = type;
        varInfo.location = location;
        varInfo.arraySize = size;

        switch (varInfo.type) {
        case GL_INT:
            glGetUniformiv(computeProgram, varInfo.location, &varInfo.value.i);
            variablesMap[varName] = varInfo;
            break;
        case GL_FLOAT:
            glGetUniformfv(computeProgram, varInfo.location, &varInfo.value.f);
            variablesMap[varName] = varInfo;
            break;
        case GL_BOOL:
            GLint tempBool;
            glGetUniformiv(computeProgram, varInfo.location, &tempBool);
            varInfo.value.b = (tempBool != 0);
            variablesMap[varName] = varInfo;
            break;
        case GL_INT_VEC2:
            GLint ivec2[2];
            glGetUniformiv(computeProgram, varInfo.location, ivec2);
            varInfo.value.i = ivec2[0];
            variablesMap[varName] = varInfo;
            break;
        default:
            std::cerr << "Warning: Unsupported uniform type: " << varInfo.type << std::endl;
            continue;
        }

        UIElement uiElement;
        uiElement.varName = varInfo.name;
        uiElement.text = varInfo.name;
        uiElement.type = (varInfo.type == GL_INT) ? UIElementType::InputInt :
            (varInfo.type == GL_FLOAT) ? UIElementType::InputFloat :
            (varInfo.type == GL_BOOL) ? UIElementType::InputBool :
            (varInfo.type == GL_INT_VEC2) ? UIElementType::InputInt : UIElementType::InputInt;
        uiElement.intValue = (varInfo.type == GL_INT) ? varInfo.value.i :
            (varInfo.type == GL_FLOAT) ? static_cast<int>(varInfo.value.f) :
            (varInfo.type == GL_BOOL) ? varInfo.value.b : varInfo.value.i;
        uiElements.push_back(uiElement);
    }

    ModManager::createModUIFromElements(uiElements);
}


void ModSystemAutomaton::updateShaderUniforms() {
    if (computeProgram == 0) {
        std::cerr << "Ошибка: Шейдерная программа не найдена: " << computeProgram << std::endl;
        return;
    }

    glUseProgram(computeProgram);
    // Обновляем униформы значениями из UI (ModManager -> UIBuilder)
    for (const auto& pair : variablesMap) {
        const ShaderVariableInfo& var = pair.second;
        if (var.location < 0) continue;

        std::cout << "Обновление униформа: " << var.name << " Тип (hex): 0x" << std::hex << var.type
            << " Тип (dec): " << std::dec << (int)var.type << " Местоположение: " << var.location << std::endl;

        // Получаем значения из UI вместо использования сохранённого var.value
        switch (var.type) {
        case GL_INT: {
            int v = ModManager::getModIntValue(var.name);
            glUniform1i(var.location, v);
            break;
        }
        case GL_FLOAT: {
            float f = ModManager::getModFloatValue(var.name);
            glUniform1f(var.location, f);
            break;
        }
        case GL_BOOL: {
            bool b = ModManager::getModBoolValue(var.name);
            glUniform1i(var.location, b ? 1 : 0);
            break;
        }
        case GL_INT_VEC2: {
            int v0 = ModManager::getModIntValue(var.name);
            glUniform2i(var.location, v0, 0);
            break;
        }
        default:
            std::cerr << "Ошибка: Неизвестный тип униформа: " << var.type << std::endl;
            break;
        }
    }
}