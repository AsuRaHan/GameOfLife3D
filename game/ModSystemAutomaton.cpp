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
        std::cerr << "Error: Shader program not found" << std::endl;
        return;
    }

    variablesMap.clear();
    std::vector<UIElement> uiElements;

    GLint numUniforms;
    glGetProgramiv(computeProgram, GL_ACTIVE_UNIFORMS, &numUniforms);
    std::cout << "Number of active uniforms: " << numUniforms << std::endl;

    // Список системных переменных, которые нужно пропускать
    std::unordered_set<std::string> systemVariables = {
        "gridSize",
        "isToroidal",
        "neighborhoodRadius",
        "birth",
        "survivalMin",
        "survivalMax",
        "overpopulation",
        "birthCounts",
        "surviveCounts",
        "overpopulationCounts",
        "useAdvancedRules"
    };

    for (int i = 0; i < numUniforms; ++i) {
        GLchar name[256];
        GLint size;
        GLenum type;
        glGetActiveUniform(computeProgram, i, 256, NULL, &size, &type, name);
        GLint location = glGetUniformLocation(computeProgram, name);

        std::string varName = std::string(name);
        
        // Пропускаем системные переменные
        if (varName.rfind("gl_", 0) == 0) continue;
        if (systemVariables.find(varName) != systemVariables.end()) {
            std::cout << "Skipping system variable: " << varName << std::endl;
            continue;
        }

        ShaderVariableInfo varInfo;
        varInfo.name = varName;
        varInfo.type = type;
        varInfo.location = location;
        varInfo.arraySize = size;

        UIElement uiElement;
        uiElement.varName = varName;
        uiElement.text = varName;

        switch (type) {
        case GL_INT: {
            GLint intValue;
            glGetUniformiv(computeProgram, location, &intValue);
            varInfo.value.i = intValue;

            uiElement.type = UIElementType::InputInt;
            uiElement.intValue = intValue;
            break;
        }
        case GL_FLOAT: {
            GLfloat floatValue;
            glGetUniformfv(computeProgram, location, &floatValue);
            varInfo.value.f = floatValue;

            uiElement.type = UIElementType::InputFloat;
            uiElement.floatValue = floatValue;
            break;
        }
        case GL_BOOL: {
            GLint boolValue;
            glGetUniformiv(computeProgram, location, &boolValue);
            varInfo.value.b = (boolValue != 0);

            uiElement.type = UIElementType::InputBool;
            uiElement.intValue = (boolValue != 0) ? 1 : 0;
            break;
        }
        case GL_INT_VEC2: {
            // Проверяем, не системная ли это переменная (например, gridSize)
            if (varName == "gridSize") {
                std::cout << "Skipping gridSize (handled by system)" << std::endl;
                continue;
            }
            
            GLint vec2[2];
            glGetUniformiv(computeProgram, location, vec2);

            varInfo.values.push_back((float)vec2[0]);
            varInfo.values.push_back((float)vec2[1]);
            varInfo.value.i = vec2[0];

            uiElement.type = UIElementType::InputInt;
            uiElement.intValue = vec2[0];

            UIElement uiElementY;
            uiElementY.varName = varName + "_y";
            uiElementY.text = varName + " Y";
            uiElementY.type = UIElementType::InputInt;
            uiElementY.intValue = vec2[1];
            uiElements.push_back(uiElementY);
            break;
        }
        case GL_FLOAT_VEC2: {
            GLfloat vec2[2];
            glGetUniformfv(computeProgram, location, vec2);

            varInfo.values.push_back(vec2[0]);
            varInfo.values.push_back(vec2[1]);
            varInfo.value.f = vec2[0];

            uiElement.type = UIElementType::InputFloat;
            uiElement.floatValue = vec2[0];

            UIElement uiElementY;
            uiElementY.varName = varName + "_y";
            uiElementY.text = varName + " Y";
            uiElementY.type = UIElementType::InputFloat;
            uiElementY.floatValue = vec2[1];
            uiElements.push_back(uiElementY);
            break;
        }
        case GL_INT_VEC3:
        case GL_INT_VEC4:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4: {
            std::cerr << "Vector uniforms not fully supported yet: " << varName << std::endl;
            continue;
        }
        default:
            std::cerr << "Warning: Unsupported uniform type: " << type
                << " for " << varName << std::endl;
            continue;
        }

        variablesMap[varName] = varInfo;
        uiElements.push_back(uiElement);
        
        // Отладочный вывод
        std::cout << "Added mod variable: " << varName 
                  << " type: 0x" << std::hex << type << std::dec
                  << " location: " << location << std::endl;
    }

    // Сохраняем системные uniform location для быстрого доступа
    cacheSystemUniformLocations();

    ModManager::createModUIFromElements(uiElements);
}

void ModSystemAutomaton::cacheSystemUniformLocations() {
    // Кэшируем location системных uniform-ов для updateShaderUniforms
    systemUniforms.clear();

    systemUniforms["gridSize"] = glGetUniformLocation(computeProgram, "gridSize");
    systemUniforms["isToroidal"] = glGetUniformLocation(computeProgram, "isToroidal");
    systemUniforms["neighborhoodRadius"] = glGetUniformLocation(computeProgram, "neighborhoodRadius");
    systemUniforms["birth"] = glGetUniformLocation(computeProgram, "birth");
    systemUniforms["survivalMin"] = glGetUniformLocation(computeProgram, "survivalMin");
    systemUniforms["survivalMax"] = glGetUniformLocation(computeProgram, "survivalMax");
    systemUniforms["overpopulation"] = glGetUniformLocation(computeProgram, "overpopulation");
    systemUniforms["useAdvancedRules"] = glGetUniformLocation(computeProgram, "useAdvancedRules");

    // Для массивов
    systemUniforms["birthCounts"] = glGetUniformLocation(computeProgram, "birthCounts");
    systemUniforms["surviveCounts"] = glGetUniformLocation(computeProgram, "surviveCounts");
    systemUniforms["overpopulationCounts"] = glGetUniformLocation(computeProgram, "overpopulationCounts");
}

void ModSystemAutomaton::updateShaderUniforms() {
    if (computeProgram == 0) {
        std::cerr << "Error: Shader program not found" << std::endl;
        return;
    }

    glUseProgram(computeProgram);

    // 1. Сначала обновляем СИСТЕМНЫЕ uniform-ы
    updateSystemUniforms();

    // 2. Затем обновляем uniform-ы МОДА из UI
    for (const auto& pair : variablesMap) {
        const ShaderVariableInfo& var = pair.second;
        if (var.location < 0) continue;

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
            int v1 = ModManager::getModIntValue(var.name + "_y");
            glUniform2i(var.location, v0, v1);
            break;
        }
        case GL_FLOAT_VEC2: {
            float v0 = ModManager::getModFloatValue(var.name);
            float v1 = ModManager::getModFloatValue(var.name + "_y");
            glUniform2f(var.location, v0, v1);
            break;
        }
        default:
            std::cerr << "Error: Unknown uniform type: " << var.type
                << " for " << var.name << std::endl;
            break;
        }
    }
}

void ModSystemAutomaton::updateSystemUniforms() {
    // Обновляем стандартные системные параметры

    if (systemUniforms["gridSize"] != -1) {
        glUniform2i(systemUniforms["gridSize"], gridWidth, gridHeight);
    }

    if (systemUniforms["isToroidal"] != -1) {
        glUniform1i(systemUniforms["isToroidal"], isToroidal ? 1 : 0);
    }

    if (systemUniforms["neighborhoodRadius"] != -1) {
        glUniform1i(systemUniforms["neighborhoodRadius"], neighborhoodRadius);
    }

    if (systemUniforms["birth"] != -1) {
        glUniform1i(systemUniforms["birth"], birth);
    }

    if (systemUniforms["survivalMin"] != -1) {
        glUniform1i(systemUniforms["survivalMin"], survivalMin);
    }

    if (systemUniforms["survivalMax"] != -1) {
        glUniform1i(systemUniforms["survivalMax"], survivalMax);
    }

    if (systemUniforms["overpopulation"] != -1) {
        glUniform1i(systemUniforms["overpopulation"], overpopulation);
    }

    if (systemUniforms["useAdvancedRules"] != -1) {
        glUniform1i(systemUniforms["useAdvancedRules"], useAdvancedRules ? 1 : 0);
    }

    // Обновляем массивы правил
    if (systemUniforms["birthCounts"] != -1) {
        glUniform1iv(systemUniforms["birthCounts"], 9, birthRules);
    }

    if (systemUniforms["surviveCounts"] != -1) {
        glUniform1iv(systemUniforms["surviveCounts"], 9, surviveRules);
    }

    if (systemUniforms["overpopulationCounts"] != -1) {
        glUniform1iv(systemUniforms["overpopulationCounts"], 9, overpopulationRules);
    }
}