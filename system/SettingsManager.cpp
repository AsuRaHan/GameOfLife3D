#include "SettingsManager.h"

SettingsManager::SettingsManager(const std::string& filename) : settingsFile(filename) {
    loadFromFile();
}

void SettingsManager::saveToFile() {
    std::ofstream file(settingsFile);
    if (file.is_open()) {
        for (const auto& [section, sectionSettings] : settings) {
            file << "[" << section << "]" << std::endl;
            for (const auto& [key, value] : sectionSettings) {
                file << key << "=" << value << std::endl;
            }
            file << std::endl; // Добавляем пустую строку для разделения секций
        }
        file.close();
    }
}

void SettingsManager::loadFromFile() {
    std::ifstream file(settingsFile);
    if (file.is_open()) {
        std::string line, currentSection;
        while (std::getline(file, line)) {
            if (line.empty()) continue; // Пропускаем пустые строки
            if (line[0] == '[' && line[line.length() - 1] == ']') { // Начало новой секции
                currentSection = line.substr(1, line.length() - 2);
            }
            else if (!currentSection.empty()) { // Заполнение текущей секции
                std::istringstream iss(line);
                std::string key, value;
                if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                    settings[currentSection][key] = value;
                }
            }
        }
        file.close();
    }
}

void SettingsManager::setSetting(const std::string& section, const std::string& key, const std::string& value) {
    settings[section][key] = value;
    saveToFile();
}

void SettingsManager::setSetting(const std::string& section, const std::string& key, int value) {
    setSetting(section, key, std::to_string(value));
}

void SettingsManager::setSetting(const std::string& section, const std::string& key, float value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << value;
    setSetting(section, key, oss.str());
}

std::string SettingsManager::getSetting(const std::string& section, const std::string& key) {
    auto itSection = settings.find(section);
    if (itSection != settings.end()) {
        auto it = itSection->second.find(key);
        if (it != itSection->second.end()) {
            return it->second;
        }
    }
    return "";
}

int SettingsManager::getIntSetting(const std::string& section, const std::string& key, int defaultValue) {
    std::string value = getSetting(section, key);
    if (value.empty()) return defaultValue;
    try {
        return std::stoi(value);
    }
    catch (const std::exception&) {
        return defaultValue;
    }
}

float SettingsManager::getFloatSetting(const std::string& section, const std::string& key, float defaultValue) {
    std::string value = getSetting(section, key);
    if (value.empty()) return defaultValue;
    try {
        return std::stof(value);
    }
    catch (const std::exception&) {
        return defaultValue;
    }
}

void SettingsManager::removeSetting(const std::string& section, const std::string& key) {
    auto sectionIt = settings.find(section);
    if (sectionIt != settings.end()) {
        auto it = sectionIt->second.find(key);
        if (it != sectionIt->second.end()) {
            sectionIt->second.erase(it);
            saveToFile();
        }
    }
}

void SettingsManager::setSetting(const std::string& section, const std::string& key, bool value) {
    settings[section][key] = value ? "true" : "false";
    saveToFile();
}

bool SettingsManager::getBoolSetting(const std::string& section, const std::string& key, bool defaultValue) {
    std::string value = getSetting(section, key);
    if (value.empty()) return defaultValue;
    return (value == "true" || value == "1");
}

void SettingsManager::loadImGuiSettings() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.IniFilename == NULL) {
        io.IniFilename = settingsFile.c_str(); // Убедитесь, что используется один файл
    }
    ImGui::LoadIniSettingsFromDisk(io.IniFilename);
}

void SettingsManager::saveImGuiSettings() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.IniFilename == NULL) {
        io.IniFilename = settingsFile.c_str();
    }

    size_t out_size = 0;
    char* data = (char*)ImGui::SaveIniSettingsToMemory(&out_size); // Приводим к char*

    if (data != NULL) {
        std::ofstream out(settingsFile, std::ios::out | std::ios::trunc);
        if (out.is_open()) {
            out.write(data, out_size);

            // Затем сохраняем ваши настройки
            for (const auto& [section, sectionSettings] : settings) {
                out << "[" << section << "]" << std::endl;
                for (const auto& [key, value] : sectionSettings) {
                    out << key << "=" << value << std::endl;
                }
                out << std::endl;
            }
        }
        out.close();

        ImGui::MemFree(data); // Освобождаем память
    }
}