// UIBuilder.cpp
#include "UIBuilder.h"
#include <iostream>

UIBuilder::UIBuilder() {
    // Инициализация, если нужно
}

void UIBuilder::setConfig(const std::vector<UIElement>& config) {
    elements = config;
    stringVariables.clear(); // Очищаем карту переменных
    intVariables.clear();
    floatVariables.clear();
    boolVariables.clear();
    // Инициализируем переменные значениями по умолчанию
    for (const auto& element : elements) {
        if (element.type == InputText) {
            stringVariables[element.varName] = element.value;
        }
        else if (element.type == InputInt) {
            intVariables[element.varName] = element.intValue;
        }
        else if (element.type == InputFloat) {
            floatVariables[element.varName] = element.floatValue;
        }
        else if (element.type == InputBool) {
            boolVariables[element.varName] = element.intValue;
        }
    }
}

bool UIBuilder::loadConfigFromFile(const std::string& filename) {
    elements.clear();
    stringVariables.clear();
    intVariables.clear();
    floatVariables.clear();
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        UIElement element = parseLine(line);

        if (!element.varName.empty()) {
            if (element.type == InputText) {
                stringVariables[element.varName] = element.value;
                addElement(element);
            }
            else if (element.type == InputInt) {
                intVariables[element.varName] = element.intValue;
                addElement(element);
            }
            else if (element.type == InputFloat) {
                floatVariables[element.varName] = element.floatValue;
                addElement(element);
            }
        }
        else {
            addElement(element);
        }
    }

    file.close();
    return true;
}

UIElement UIBuilder::parseLine(const std::string& line) {
    UIElement element;
    element.id = -1; // Недействительный id по умолчанию
    element.type = UIElementType::Separator;

    std::istringstream iss(line);
    std::string key;
    std::string value;

    while (std::getline(iss, key, ';')) {
        size_t eqPos = key.find('=');
        if (eqPos != std::string::npos) {
            std::string keyName = trim(key.substr(0, eqPos));
            std::string keyValue = trim(key.substr(eqPos + 1));

            if (keyName == "type") {
                if (keyValue == "Separator") {
                    element.type = UIElementType::Separator;
                }
                else if (keyValue == "InputText") {
                    element.type = UIElementType::InputText;
                }
                else if (keyValue == "InputInt") {
                    element.type = UIElementType::InputInt;
                }
                else if (keyValue == "InputFloat") {
                    element.type = UIElementType::InputFloat;
                }
                else if (keyValue == "Text") {
                    element.type = UIElementType::Text;
                }
            }
            else if (keyName == "varName") {
                element.varName = keyValue;
            }
            else if (keyName == "text") {
                element.text = keyValue;
            }
            else if (keyName == "value") {
                element.value = keyValue;
                // Попытаться преобразовать в int или float, если это InputInt или InputFloat
                if (element.type == InputInt) {
                    try {
                        element.intValue = std::stoi(keyValue);
                    }
                    catch (const std::invalid_argument& e) {
                        std::cerr << "Warning: Invalid int value in config: " << keyValue << std::endl;
                    }
                }
                else if (element.type == InputFloat) {
                    try {
                        element.floatValue = std::stof(keyValue);
                    }
                    catch (const std::invalid_argument& e) {
                        std::cerr << "Warning: Invalid float value in config: " << keyValue << std::endl;
                    }
                }
            }
        }
    }
    return element;
}

std::string UIBuilder::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void UIBuilder::addElement(const UIElement& element) {
    UIElement newElement = element;
    newElement.id = nextElementId++;
    elements.push_back(newElement);
}

void UIBuilder::removeElement(int id) {
    for (size_t i = 0; i < elements.size(); ++i) {
        if (elements[i].id == id) {
            elements.erase(elements.begin() + i);
            return;
        }
    }
}

void UIBuilder::drawUI(bool* p_open) {
    ImGui::Begin(windowName.c_str(), p_open); // Начинаем окно
    for (UIElement& element : elements) {
        if (element.type == UIElementType::Separator) {
            ImGui::Separator();
        }
        else if (element.type == UIElementType::InputText) {
            ImGui::Text("%s", element.text.c_str());
            if (stringVariables.count(element.varName) > 0) {
                if (ImGui::InputText(("##" + element.varName).c_str(), &stringVariables[element.varName][0], stringVariables[element.varName].capacity() + 1)) {
                }
            }
        }
        else if (element.type == UIElementType::InputInt) {
            ImGui::Text("%s", element.text.c_str());
            if (intVariables.count(element.varName) > 0) {
                ImGui::InputInt(("##" + element.varName).c_str(), &intVariables[element.varName]);
            }
        }
        else if (element.type == UIElementType::InputFloat) {
            ImGui::Text("%s", element.text.c_str());
            if (floatVariables.count(element.varName) > 0) {
                ImGui::InputFloat(("##" + element.varName).c_str(), &floatVariables[element.varName]);
            }
        }
        else if (element.type == UIElementType::InputBool) {
            ImGui::Text("%s", element.text.c_str());
            if (boolVariables.count(element.varName) > 0) {
                ImGui::Checkbox(("##" + element.varName).c_str(), &boolVariables[element.varName]);
            }
        }
        else if (element.type == UIElementType::Text){
             ImGui::Text("%s", element.text.c_str());
        }
    }
    ImGui::End(); // Заканчиваем окно
}

const std::vector<UIElement>& UIBuilder::getElements() const {
    return elements;
}

const std::string& UIBuilder::getStringElementValue(const std::string& varName) const {
    static const std::string empty = "";
    if (stringVariables.count(varName) > 0) {
        return stringVariables.at(varName);
    }
    return empty;
}

int UIBuilder::getIntElementValue(const std::string& varName) const {
    if (intVariables.count(varName) > 0) {
        return intVariables.at(varName);
    }
    return 0; // Возвращаем значение по умолчанию
}

float UIBuilder::getFloatElementValue(const std::string& varName) const {
    if (floatVariables.count(varName) > 0) {
        return floatVariables.at(varName);
    }
    return 0.0f; // Возвращаем значение по умолчанию
}

void UIBuilder::setStringElementValue(const std::string& varName, const std::string& value) {
    stringVariables[varName] = value;
}

void UIBuilder::setIntElementValue(const std::string& varName, int value) {
    intVariables[varName] = value;
}

void UIBuilder::setFloatElementValue(const std::string& varName, float value) {
    floatVariables[varName] = value;
}

void UIBuilder::setWindowName(const std::string& name) {
    windowName = name;
}
