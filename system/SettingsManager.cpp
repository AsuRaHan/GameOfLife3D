#include "SettingsManager.h"
#include <fstream>

SettingsManager::SettingsManager(const std::string& filename) : settingsFile(filename) {
    loadFromFile();
}

void SettingsManager::saveToFile() {
    std::ofstream file(settingsFile);
    if (file.is_open()) {
        for (const auto& pair : settings) {
            file << pair.first << "=" << pair.second << std::endl;
        }
        file.close();
    }
}

void SettingsManager::loadFromFile() {
    std::ifstream file(settingsFile);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key, value;
            if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                settings[key] = value;
            }
        }
        file.close();
    }
}

void SettingsManager::setSetting(const std::string& key, const std::string& value) {
    settings[key] = value;
    saveToFile();
}

void SettingsManager::setSetting(const std::string& key, int value) {
    setSetting(key, std::to_string(value));
}

void SettingsManager::setSetting(const std::string& key, float value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << value; // Используем фиксированную точность для float
    setSetting(key, oss.str());
}

std::string SettingsManager::getSetting(const std::string& key) {
    auto it = settings.find(key);
    return it != settings.end() ? it->second : "";
}

int SettingsManager::getIntSetting(const std::string& key, int defaultValue) {
    std::string value = getSetting(key);
    if (value.empty()) return defaultValue;
    try {
        return std::stoi(value);
    }
    catch (const std::exception&) {
        return defaultValue;
    }
}

float SettingsManager::getFloatSetting(const std::string& key, float defaultValue) {
    std::string value = getSetting(key);
    if (value.empty()) return defaultValue;
    try {
        return std::stof(value);
    }
    catch (const std::exception&) {
        return defaultValue;
    }
}

void SettingsManager::removeSetting(const std::string& key) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        settings.erase(it);
        saveToFile();
    }
}

void SettingsManager::setSetting(const std::string& key, bool value) {
    settings[key] = value ? "true" : "false"; // Сохраняем значение в settings
    saveToFile(); // Сохраняем изменения в файл
}

bool SettingsManager::getBoolSetting(const std::string& key, bool defaultValue) {
    std::string value = getSetting(key);
    if (value.empty()) return defaultValue;
    return (value == "true" || value == "1"); // Считаем "true" или "1" за true, остальное - за false
}