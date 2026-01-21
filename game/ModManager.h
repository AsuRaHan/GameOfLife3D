// ModManager.h
#pragma once
#ifndef MODMANAGER_H
#define MODMANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem> // Для работы с файловой системой
#include <cstring>    // для memcmp
#include <map>        // для хранения параметров мода
#include <algorithm> // Для std::transform

#include "UIBuilder.h"

class ModManager {
public:
    // Структура для хранения параметров мода
    struct ModConfig {
        std::string modName;
        int minWorldSizeX;
        int minWorldSizeY;
        int minWorldSizeZ;
        std::vector<std::string> cellTypeNames; // Массив имен типов клеток
    };
    // Список файлов, которые должны быть в папке "mods" (базовые)
    static const std::vector<std::string> requiredModFiles;

    // Список доступных модов (названия папок)
    static std::vector<std::string> availableMods;

    // Карта для хранения конфигураций модов
    static std::map<std::string, ModConfig> modConfigs;

    // Метод для проверки наличия папки "mods" и необходимых файлов в ней
    static bool checkMods();

    // Метод для получения пути к папке "mods"
    static std::string getModsFolderPath();

    // метод получения пути к файлу
    static std::string getModFilePath(const std::string& filename);

    // Метод для загрузки списка доступных модов из mod_list.gmod
    static void loadAvailableMods();

    // Метод для загрузки конфигурации мода из mod.cfg
    static ModConfig loadModConfig(const std::string& modFolderName);

    // Getter для имени текущего мода
    static const std::string& getCurrentModName();

    // Setter для имени текущего мода
    static void setCurrentModName(const std::string& modName);

    // Getter для получения списка доступных модов
    static const std::vector<std::string>& getAvailableMods();

    // Отрисовка UI для текущего мода
    static void drawCurrentModUI();

    static const std::vector<UIElement>& getCurrentModUIElements();
    static void createModUIFromElements(const std::vector<UIElement>& elements);

    // Helpers to get current values from mod UI
    static const std::string& getModStringValue(const std::string& varName);
    static int getModIntValue(const std::string& varName);
    static float getModFloatValue(const std::string& varName);
    static bool getModBoolValue(const std::string& varName);

private:
    // Имя текущего мода
    static std::string currentModName;
         // Билдер ui
    static UIBuilder modUIBuilder;
};

#endif // MODMANAGER_H
