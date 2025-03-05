// UIBuilder.h
#pragma once
#ifndef UI_BUILDER_H
#define UI_BUILDER_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include "imgui.h"
#include "UIElement.h"

class UIBuilder {
public:
    // Конструктор
    UIBuilder();

    // Метод для установки конфигурации интерфейса
    void setConfig(const std::vector<UIElement>& config);

    // Метод для загрузки конфигурации из файла
    bool loadConfigFromFile(const std::string& filename);

    // Метод для добавления элемента
    void addElement(const UIElement& element);

    // Метод для удаления элемента
    void removeElement(int id);

    // Метод для отрисовки интерфейса
    void drawUI(bool* p_open = nullptr); // Изменен drawUI

    //Метод для получения списка элементов
    const std::vector<UIElement>& getElements() const;

    // Методы для получения значения элемента
    const std::string& getStringElementValue(const std::string& varName) const;
    int getIntElementValue(const std::string& varName) const;
    float getFloatElementValue(const std::string& varName) const;

    // Методы для установки значения элемента
    void setStringElementValue(const std::string& varName, const std::string& value);
    void setIntElementValue(const std::string& varName, int value);
    void setFloatElementValue(const std::string& varName, float value);
    // метод для установки имени окна
    void setWindowName(const std::string& name);

private:
    std::vector<UIElement> elements; // Список всех элементов интерфейса
    int nextElementId = 0; // счетчик для id
    
    std::map<std::string, std::string> stringVariables; // Карта для хранения строковых значений переменных
    std::map<std::string, int> intVariables;             // Карта для хранения целочисленных значений переменных
    std::map<std::string, bool> boolVariables;             // Карта для хранения bool значений переменных
    std::map<std::string, float> floatVariables;         // Карта для хранения вещественных значений переменных


    std::string windowName = "MOD UI Window"; // имя окна.

    UIElement parseLine(const std::string& line);
    std::string trim(const std::string& str);

};

#endif // UI_BUILDER_H
