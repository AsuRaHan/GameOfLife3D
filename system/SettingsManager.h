#pragma once
#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h" // Если вы работаете под Windows

class SettingsManager {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> settings;
    std::string settingsFile;
public:
    void saveToFile();
    void loadFromFile();
    void loadImGuiSettings();
    void saveImGuiSettings();

    SettingsManager(const std::string& filename = "settings.ini");
    void setSetting(const std::string& section, const std::string& key, const std::string& value);
    void setSetting(const std::string& section, const std::string& key, int value);
    void setSetting(const std::string& section, const std::string& key, float value);
    void setSetting(const std::string& section, const std::string& key, bool value);

    std::string getSetting(const std::string& section, const std::string& key);
    int getIntSetting(const std::string& section, const std::string& key, int defaultValue = 0);
    float getFloatSetting(const std::string& section, const std::string& key, float defaultValue = 0.0f);
    bool getBoolSetting(const std::string& section, const std::string& key, bool defaultValue = false);

    void removeSetting(const std::string& section, const std::string& key);
};

#endif // SETTINGS_MANAGER_H