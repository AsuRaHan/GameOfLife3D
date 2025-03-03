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

    LoadSelectedMod();
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
}


std::vector<ShaderVariableInfo> ModSystemAutomaton::getModShaderVariables() {
    std::vector<ShaderVariableInfo> variables;

    // Получаем имя текущего мода
    std::string currentModName = ModManager::getCurrentModName();
    if (currentModName.empty()) {
        currentModName = "modystemshader";
    }

    // Формируем имя программы на основе имени мода
    std::string programName = currentModName + "Program";

    // Получаем ID программы шейдера
    GLuint programId = shaderManager.getProgram(programName);

    if (programId == 0) {
        std::cerr << "Error: Shader program not found: " << programName << std::endl;
        return variables; // Возвращаем пустой список, если программа не найдена
    }

    GLint numUniforms;
    glGetProgramInterfaceiv(programId, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_ARRAY_SIZE };
    const GLsizei numProps = sizeof(properties) / sizeof(properties[0]);

    for (int i = 0; i < numUniforms; ++i) {
        GLint values[numProps];
        glGetProgramResourceiv(programId, GL_UNIFORM, i, numProps, properties, numProps, NULL, values);

        GLint nameLength = values[0];
        GLchar* name = new GLchar[nameLength];
        glGetProgramResourceName(programId, GL_UNIFORM, i, nameLength, NULL, name);

        ShaderVariableInfo varInfo;
        varInfo.name = std::string(name);
        varInfo.location = values[2];
        varInfo.type = values[1];
        varInfo.arraySize = values[3];

         // Initialize the vector to the correct size if it's an array
        if (varInfo.arraySize > 0) {
             varInfo.values.resize(varInfo.arraySize,0.0f);
        }

        // Фильтруем переменные
        if (varInfo.name != "current" && varInfo.name != "next" && varInfo.name != "colors" && varInfo.name != "gl_NumWorkGroups" && varInfo.name != "gl_WorkGroupSize" && varInfo.name != "gl_WorkGroupID" && varInfo.name != "gl_LocalInvocationID" && varInfo.name != "gl_GlobalInvocationID" && varInfo.name != "gridSize")
            variables.push_back(varInfo);

        delete[] name;
    }

    return variables;
}


void ModSystemAutomaton::updateShaderUniforms() {
    std::string currentModName = ModManager::getCurrentModName();
    if (currentModName.empty()) {
        currentModName = "modystemshader";
    }
    std::string programName = currentModName + "Program";
    GLuint programId = shaderManager.getProgram(programName);

    if (programId == 0) {
        std::cerr << "Error: Shader program not found: " << programName << std::endl;
        return;
    }

    glUseProgram(programId);
    addUIElementsToVariablesMap();
    for (const auto& pair : variablesMap) {
        const ShaderVariableInfo* var = pair.second;
        if(var->location < 0) continue;
        switch (var->type) {
        case GL_INT:
        case GL_SAMPLER_2D://добавляем int
            glUniform1i(var->location, var->value.i);
            break;
        case GL_FLOAT:
            glUniform1f(var->location, var->value.f);
            break;
        case GL_BOOL:
            glUniform1i(var->location, var->value.b);
            break;
        case GL_FLOAT_VEC4: //не будет vec4
        case GL_INT_VEC2: //не будет vec2i
         // Check if it's an array
            if (var->arraySize > 0) {
                 if (var->type == GL_FLOAT_VEC4) {
                    //переделать под float массив
                } else {
                     //отправляем как int[]
                    glUniform1iv(var->location, var->arraySize, (GLint*)var->values.data());

                 }
            }
            break;
        case GL_FLOAT_VEC3:
         case GL_INT_VEC3:
         case GL_INT_VEC4:
        case GL_FLOAT_VEC2:
            std::cerr << "Error: Not support vec3, vec2, vec4i uniform type: " << var->type << std::endl;
            break;
        default:
            std::cerr << "Error: Unknown uniform type: " << var->type << std::endl;
            break;
        }
    }
}

void ModSystemAutomaton::addUIElementsToVariablesMap() {
    // Получаем список элементов UI из ModManager
    const std::vector<UIElement>& uiElements = ModManager::getCurrentModUIElements();

    // Проходим по списку элементов UI
    for (const auto& uiElement : uiElements) {
        // Игнорируем разделители и просто текст.
        if (uiElement.type == UIElementType::Separator || uiElement.type == UIElementType::Text) continue;

        // Проверяем, есть ли уже такая переменная в variablesMap
        if (variablesMap.find(uiElement.varName) != variablesMap.end()) {
            std::cerr << "Warning: Variable already exists: " << uiElement.varName << std::endl;
            continue; // Пропускаем дубликаты
        }

        // Создаем ShaderVariableInfo
        ShaderVariableInfo varInfo;
        varInfo.name = uiElement.varName;
        varInfo.location = glGetUniformLocation(computeProgram, uiElement.varName.c_str());// -1; // Устанавливаем -1, так как у нас нет информации о location (не из шейдера)
        varInfo.arraySize = 0;//по умочанию

        // Определяем тип переменной и присваиваем значение по умолчанию
        if (uiElement.type == UIElementType::InputInt) {
            varInfo.type = GL_INT;
            varInfo.value.i = uiElement.intValue;
            varInfo.valueStr = std::to_string(uiElement.intValue);
        }
        else if (uiElement.type == UIElementType::InputFloat) {
            varInfo.type = GL_FLOAT;
            varInfo.value.f = uiElement.floatValue;
            varInfo.valueStr = std::to_string(uiElement.floatValue);
        }
        else if (uiElement.type == UIElementType::InputText) {
            varInfo.type = GL_NONE; // или другой тип для текстового поля
            varInfo.valueStr = uiElement.value;
        }
        else {
            std::cerr << "Error: Unsupported variable type: " << uiElement.type << std::endl;
            continue; // Пропускаем неподдерживаемые типы
        }

        // Создаем callback функцию
        varInfo.callback = [this](const ShaderVariableInfo& varInfo, std::string newVar) {
            // Проверяем, есть ли переменная в map
            auto it = variablesMap.find(varInfo.name);
            if (it != variablesMap.end()) {
                ShaderVariableInfo* varPtr = it->second;
                // Получаем тип переменной и конвертируем значение
                switch (varPtr->type)
                {
                case GL_INT:
                case GL_SAMPLER_2D: //добавляем int
                    varPtr->value.i = std::stoi(newVar);
                    break;
                case GL_FLOAT:
                    varPtr->value.f = std::stof(newVar);
                    break;
                case GL_BOOL:
                    varPtr->value.b = (newVar == "true") ? true : false;
                    break;
                case GL_NONE:
                    varPtr->valueStr = newVar;
                    break;
                default:
                    std::cerr << "Error: Unknown uniform type: " << varPtr->type << std::endl;
                    break;
                }
                varPtr->valueStr = newVar;//обновляем значение
            }
            else {
                std::cerr << "Error: Not find varibale " << varInfo.name << std::endl;
            }

            };

        // Добавляем переменную в map
        variablesMap[varInfo.name] = &varInfo;
    }
}