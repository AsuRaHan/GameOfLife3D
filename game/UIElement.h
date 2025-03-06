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
//struct ShaderVariableInfo {
//    std::string name;
//    GLint location;
//    GLenum type;
//    GLsizei arraySize;
//    union Value {
//        int i;
//        float f;
//        bool b;
//        Value() : i(0) {} // Конструктор по умолчанию
//    } value;
//    std::vector<float> values; // Для массивов
//    std::string valueStr; // Для текста
//};
struct ShaderVariableInfo {
    std::string name;
    GLint location;
    GLenum type;
    GLsizei arraySize;
    union Value {
        int i;
        float f;
        bool b;
        Value() : i(0) {} // Конструктор по умолчанию
    } value;
    std::vector<float> values; // Для массивов
    std::string valueStr; // Для текста

    ShaderVariableInfo() : name(""), location(0), type(0), arraySize(0), value(), values(), valueStr("") {}
};
#endif // UI_ELEMENT_H
