// UIElement.h
#pragma once
#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H
#include "../system/GLFunctions.h"
#include <string>
#include <vector>
#include <functional> // Для std::function

// Перечисление для типов элементов
enum UIElementType {
    Separator,
    InputText,
    InputInt,
    InputFloat,
    Text,
    InputBool
};

// Структура для элемента интерфейса
struct UIElement {
    int id = -1;                // Уникальный ID элемента
    UIElementType type = UIElementType::Separator;    // Тип элемента
    std::string varName = "";   // Имя переменной, с которой связан элемент
    std::string text = "";      // Текст, отображаемый рядом с элементом
    std::string value = "";     // Текущее значение элемента (для InputText)
    int intValue = 0;        // Текущее значение элемента (для InputInt)
    float floatValue = 0.0f; // Текущее значение элемента (для InputFloat)
};

// Структура для информации о переменной шейдера
struct ShaderVariableInfo {
    std::string name;
    GLint location;
    GLenum type;
    GLint arraySize;
    std::function<void(const ShaderVariableInfo&, std::string)> callback; // Callback function
    // Убираем glm::vec4 и glm::ivec2
    union {
        int i;
        float f;
        bool b;
    } value;
    std::string valueStr = ""; // value in str
    // Add max min value
    float min = 0.0f;
    float max = 0.0f;
    
    // Добавляем массив для хранения значений, если это массив
    std::vector<float> values; 
};

#endif // UI_ELEMENT_H
