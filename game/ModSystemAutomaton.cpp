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

    glGetProgramInterfaceiv(computeProgram, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_ARRAY_SIZE };
    const GLsizei numProps = sizeof(properties) / sizeof(properties[0]);

    for (int i = 0; i < numUniforms; ++i) {
        GLint values[numProps];
        glGetProgramResourceiv(computeProgram, GL_UNIFORM, i, numProps, properties, numProps, NULL, values);

        GLint nameLength = values[0];
        GLchar* name = new GLchar[nameLength];
        glGetProgramResourceName(computeProgram, GL_UNIFORM, i, nameLength, NULL, name);
        std::string varName = std::string(name);

        if (nameLength > 1 && varName.rfind("gl_", 0) != 0) {
            ShaderVariableInfo varInfo;
            varInfo.name = varName;
            varInfo.type = values[1];
            varInfo.location = values[2];            
            varInfo.arraySize = values[3];

            //Skip not supported
            //if (varInfo.arraySize > 0) continue;
            if (varInfo.type != GL_INT && varInfo.type != GL_FLOAT && varInfo.type != GL_BOOL) continue;

            //if (varInfo.arraySize > 0) {
            //    varInfo.values.resize(varInfo.arraySize, 0.0f);
            //}
            UIElement uiElement;
            uiElement.varName = varInfo.name;
            uiElement.text = varInfo.name;

            switch (varInfo.type) {
            case GL_INT:
            //case GL_SAMPLER_2D:
                glGetUniformiv(computeProgram, varInfo.location, &varInfo.value.i);
                variablesMap[varName] = new ShaderVariableInfo(varInfo);
                uiElement.type = UIElementType::InputInt;
                uiElement.intValue = varInfo.value.i;

                break;
            case GL_FLOAT:
                glGetUniformfv(computeProgram, varInfo.location, &varInfo.value.f);
                variablesMap[varName] = new ShaderVariableInfo(varInfo);
                uiElement.type = UIElementType::InputFloat;
                uiElement.floatValue = varInfo.value.f;                
                break;
            case GL_BOOL:
                GLint tempBool;
                glGetUniformiv(computeProgram, varInfo.location, &tempBool);
                varInfo.value.b = (tempBool != 0);
                variablesMap[varName] = new ShaderVariableInfo(varInfo);
                uiElement.type = UIElementType::InputBool;
                uiElement.intValue = varInfo.value.b;
                break;
            default:
                std::cerr << "Warning: Unsupported uniform type for reading: " << varInfo.type << std::endl;
                break;
            }
            uiElements.push_back(uiElement);
        }
        
        delete[] name;
    }

    ModManager::createModUIFromElements(uiElements);
}



void ModSystemAutomaton::updateShaderUniforms() {
    if (computeProgram == 0) {
        std::cerr << "Error: Shader program not found: " << computeProgram << std::endl;
        return;
    }

    for (const auto& pair : variablesMap) {
        const ShaderVariableInfo* var = pair.second;
        if(var->location < 0) continue;
        switch (var->type) {
        case GL_INT://добавляем int
        //case GL_SAMPLER_2D:
            glUniform1i(glGetUniformLocation(computeProgram, var->name.c_str()), var->value.i);
            break;
        case GL_FLOAT:
            glUniform1f(glGetUniformLocation(computeProgram, var->name.c_str()), var->value.f);
            break;
        case GL_BOOL:
            glUniform1i(glGetUniformLocation(computeProgram, var->name.c_str()), var->value.b);
            break;
        default:
            std::cerr << "Error: Unknown uniform type: " << var->type << std::endl;
            break;
        }
    }
}
