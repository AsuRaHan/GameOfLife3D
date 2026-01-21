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

    for (int i = 0; i < numUniforms; ++i) {
        GLchar name[256];
        GLint size;
        GLenum type;
        glGetActiveUniform(computeProgram, i, 256, NULL, &size, &type, name);
        GLint location = glGetUniformLocation(computeProgram, name);

        std::string varName = std::string(name);
        if (varName.rfind("gl_", 0) == 0) continue;

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
            uiElement.intValue = (boolValue != 0) ? 1 : 0; // UIBuilder использует int для bool
            break;
        }
        case GL_INT_VEC2: {
            GLint vec2[2];
            glGetUniformiv(computeProgram, location, vec2);

            // Сохраняем оба значения
            varInfo.values.push_back((float)vec2[0]);
            varInfo.values.push_back((float)vec2[1]);
            varInfo.value.i = vec2[0]; // Первое значение как основной

            // Создаём ДВА UI элемента для x и y
            uiElement.type = UIElementType::InputInt; // X компонент
            uiElement.intValue = vec2[0];

            UIElement uiElementY; // Y компонент
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

            // Два float элемента
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
            // Аналогично для других векторов
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
    }

    ModManager::createModUIFromElements(uiElements);
}


void ModSystemAutomaton::updateShaderUniforms() {
    if (computeProgram == 0) {
        std::cerr << "Ошибка: Шейдерная программа не найдена" << std::endl;
        return;
    }

    glUseProgram(computeProgram);

    for (const auto& pair : variablesMap) {
        const ShaderVariableInfo& var = pair.second;
        if (var.location < 0) continue;

        switch (var.type) {
        case GL_INT: {
            int v = ModManager::getModIntValue(var.name);
            glUniform1i(var.location, v);
            std::cout << "Set " << var.name << " = " << v << std::endl;
            break;
        }
        case GL_FLOAT: {
            float f = ModManager::getModFloatValue(var.name);
            glUniform1f(var.location, f);
            std::cout << "Set " << var.name << " = " << f << std::endl;
            break;
        }
        case GL_BOOL: {
            bool b = ModManager::getModBoolValue(var.name);
            glUniform1i(var.location, b ? 1 : 0);
            std::cout << "Set " << var.name << " = " << b << std::endl;
            break;
        }
        case GL_INT_VEC2: {
            int v0 = ModManager::getModIntValue(var.name);        // X компонент
            int v1 = ModManager::getModIntValue(var.name + "_y"); // Y компонент
            glUniform2i(var.location, v0, v1);
            std::cout << "Set " << var.name << " = [" << v0 << ", " << v1 << "]" << std::endl;
            break;
        }
        case GL_FLOAT_VEC2: {
            float v0 = ModManager::getModFloatValue(var.name);
            float v1 = ModManager::getModFloatValue(var.name + "_y");
            glUniform2f(var.location, v0, v1);
            std::cout << "Set " << var.name << " = [" << v0 << ", " << v1 << "]" << std::endl;
            break;
        }
        default:
            std::cerr << "Ошибка: Неизвестный тип униформа: " << var.type
                << " for " << var.name << std::endl;
            break;
        }
    }

    // Также обновляем системные uniform-ы
    GLint gridSizeLoc = glGetUniformLocation(computeProgram, "gridSize");
    if (gridSizeLoc != -1) {
        glUniform2i(gridSizeLoc, gridWidth, gridHeight);
    }

    GLint toroidalLoc = glGetUniformLocation(computeProgram, "isToroidal");
    if (toroidalLoc != -1) {
        glUniform1i(toroidalLoc, isToroidal ? 1 : 0);
    }
}