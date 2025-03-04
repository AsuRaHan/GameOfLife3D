// ModManager.cpp
#include "ModManager.h"
#include <algorithm> // Для std::transform

// Инициализация статических членов класса
const std::vector<std::string> ModManager::requiredModFiles = {
    "mod_list.gmod"
    // Добавьте сюда другие необходимые файлы, если они появятся
};

std::vector<std::string> ModManager::availableMods;
std::map<std::string, ModManager::ModConfig> ModManager::modConfigs;
std::string ModManager::currentModName = "modystemshader";
UIBuilder ModManager::modUIBuilder; // Инициализация modUIBuilder

const std::vector<UIElement>& ModManager::getCurrentModUIElements() { // Сделали статическим
    return modUIBuilder.getElements();
}

void ModManager::createModUIFromElements(const std::vector<UIElement>& elements) {
    modUIBuilder.setConfig(elements);
    std::cout << "UI created from elements for mod: " << currentModName << std::endl;
    modUIBuilder.setWindowName(currentModName);
}

bool ModManager::checkMods() {
    const std::string modsFolderPath = getModsFolderPath();

    // Проверяем, существует ли папка
    if (!std::filesystem::exists(modsFolderPath) || !std::filesystem::is_directory(modsFolderPath)) {
        std::cerr << "Error: 'mods' folder not found or it's not a directory." << std::endl;
        return false;
    }

    // Проверяем наличие всех необходимых файлов
    for (const auto& filename : requiredModFiles) {
        const std::string fullPath = modsFolderPath + "/" + filename;
        if (!std::filesystem::exists(fullPath) || !std::filesystem::is_regular_file(fullPath)) {
            std::cerr << "Error: Required mod file not found: " << filename << std::endl;
            return false;
        }
    }

    loadAvailableMods();

    std::cout << "Mods check successful. All required files found." << std::endl;
    return true;
}

std::string ModManager::getModsFolderPath() {
    return "./mods"; // Путь к папке "mods"
}

std::string ModManager::getModFilePath(const std::string& filename) {
    return getModsFolderPath() + "/" + filename;
}

void ModManager::loadAvailableMods() {
    availableMods.clear(); // Очищаем список перед загрузкой
    modConfigs.clear();

    const std::string modListFilename = getModsFolderPath() + "/mod_list.gmod";
    std::ifstream modListFile(modListFilename);

    if (!modListFile.is_open()) {
        std::cerr << "Error: Could not open mod list file: " << modListFilename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(modListFile, line)) {
        // Убираем пробелы в начале и в конце строки
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Игнорируем пустые строки и строки, начинающиеся с # (комментарии)
        if (!line.empty() && line[0] != '#') {
            // Проверяем, существует ли подпапка с таким именем
            std::string modFolderPath = getModsFolderPath() + "/" + line;
            if (std::filesystem::exists(modFolderPath) && std::filesystem::is_directory(modFolderPath)) {
                availableMods.push_back(line);
                std::cout << "Mod found: " << line << std::endl;
                // Загружаем конфигурацию мода
                ModManager::ModConfig config = loadModConfig(line);
                modConfigs[line] = config; // Добавляем конфигурацию в карту
            }
            else {
                std::cerr << "Warning: Mod folder not found: " << modFolderPath << std::endl;
            }
        }
    }

    modListFile.close();
    std::cout << "Total mods found: " << availableMods.size() << std::endl;
}

ModManager::ModConfig ModManager::loadModConfig(const std::string& modFolderName) {
    ModManager::ModConfig config;
    const std::string configFilename = getModsFolderPath() + "/" + modFolderName + "/mod.cfg";
    std::ifstream configFile(configFilename);

    if (!configFile.is_open()) {
        std::cerr << "Error: Could not open mod config file: " << configFilename << std::endl;
        return config; // Возвращаем пустую конфигурацию
    }

    std::string line;
    while (std::getline(configFile, line)) {
        // Удаляем пробелы в начале и в конце строки
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Игнорируем пустые строки и строки, начинающиеся с # (комментарии)
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key;
        std::string value;

        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            // Удаляем пробелы до и после '='
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            if (key == "modName") {
                config.modName = value;
            }
            else if (key == "minWorldSizeX") {
                config.minWorldSizeX = std::stoi(value);
            }
            else if (key == "minWorldSizeY") {
                config.minWorldSizeY = std::stoi(value);
            }
            else if (key == "minWorldSizeZ") {
                config.minWorldSizeZ = std::stoi(value);
            }
             else if (key == "cellTypeNames") {
                std::istringstream valueStream(value);
                std::string cellName;
                 config.cellTypeNames.clear(); // Очистите массив перед добавлением новых значений
                 while (std::getline(valueStream, cellName, ',')) {
                    cellName.erase(0, cellName.find_first_not_of(" \t"));
                    cellName.erase(cellName.find_last_not_of(" \t") + 1);
                    if (!cellName.empty()) {
                        config.cellTypeNames.push_back(cellName);
                     }
                }
            }
        }
    }

    configFile.close();
    if (config.modName.empty()) {
        config.modName = modFolderName; // если имя не указано установим как имя папки
    }
    std::cout << "Load mod config : " << config.modName << std::endl;
    return config;
}

// Getter для имени текущего мода
const std::string& ModManager::getCurrentModName() {
    return currentModName;
}

// Setter для имени текущего мода
void ModManager::setCurrentModName(const std::string& modName) {
    currentModName = modName;
}

// Getter для получения списка доступных модов
const std::vector<std::string>& ModManager::getAvailableMods() {
    return availableMods;
}

void ModManager::drawCurrentModUI() {
    // Отрисовываем UI для текущего мода
    modUIBuilder.drawUI();
}