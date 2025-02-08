#pragma once
#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

class SettingsManager {
private:
    std::unordered_map<std::string, std::string> settings;
    std::string settingsFile;

    void saveToFile();
    void loadFromFile();

public:
    SettingsManager(const std::string& filename = "settings.ini");
    void setSetting(const std::string& key, const std::string& value);
    void setSetting(const std::string& key, int value);
    void setSetting(const std::string& key, float value);
    std::string getSetting(const std::string& key);
    int getIntSetting(const std::string& key, int defaultValue = 0);
    float getFloatSetting(const std::string& key, float defaultValue = 0.0f);
    void removeSetting(const std::string& key);
    void setSetting(const std::string& key, bool value);
    bool getBoolSetting(const std::string& key, bool defaultValue = false);

};

#endif // SETTINGS_MANAGER_H