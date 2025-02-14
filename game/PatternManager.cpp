#include "PatternManager.h"
#include "GameController.h"

// Вспомогательная функция для поиска максимального значения без std::max
template<typename T>
T findMax(T a, T b) {
    return a > b ? a : b;
}

// Вспомогательная функция для поиска минимального значения без std::min
template<typename T>
T findMin(T a, T b) {
    return a < b ? a : b;
}

Pattern PatternManager::LoadPatternFromCells(const std::string& filename) const {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл: " + filename);
    }

    std::vector<std::string> patternLines;
    std::string line;

    // Читаем файл построчно
    while (std::getline(file, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '!') {
            continue;
        }
        patternLines.push_back(line);
    }

    file.close();

    if (patternLines.empty()) {
        throw std::runtime_error("Файл не содержит паттерна: " + filename);
    }

    // Определяем размеры паттерна
    size_t height = patternLines.size();
    size_t width = 0;
    for (const auto& line : patternLines) {
        width = findMax(width, static_cast<size_t>(line.size()));
    }

    // Создаём Pattern (двумерный массив bool)
    Pattern pattern(height, std::vector<bool>(width, false));

    // Заполняем Pattern
    for (size_t y = 0; y < height; ++y) {
        const std::string& line = patternLines[y];
        for (size_t x = 0; x < line.size(); ++x) {
            if (line[x] == 'O') {
                pattern[y][x] = true; // Живая клетка
            }
            else if (line[x] == '.') {
                pattern[y][x] = false; // Мёртвая клетка
            }
            else {
                throw std::runtime_error("Недопустимый символ в файле " + filename + " на позиции (" + std::to_string(x) + "," + std::to_string(y) + ")");
            }
        }
    }

    // Обрезаем пустые строки и столбцы для оптимизации
    return TrimPattern(pattern);
}

void PatternManager::LoadAndPlacePattern(GameController& gameController, const std::string& filename, int startX, int startY) const {
    Pattern pattern = LoadPatternFromCells(filename);
    if (CanPlacePattern(gameController, pattern, startX, startY)) {
        gameController.placePattern(startX, startY, pattern);
    }
    else {
        throw std::runtime_error("Паттерн из файла " + filename + " не помещается в сетку при заданных координатах (" + std::to_string(startX) + "," + std::to_string(startY) + ")");
    }
}

bool PatternManager::CanPlacePattern(const GameController& gameController, const Pattern& pattern, int startX, int startY) const {
    int patternHeight = static_cast<int>(pattern.size());
    int patternWidth = pattern.empty() ? 0 : static_cast<int>(pattern[0].size());

    // Проверяем, помещается ли паттерн в пределах сетки
    return (startX + patternWidth <= gameController.getGridWidth() && startY + patternHeight <= gameController.getGridHeight());
}

Pattern PatternManager::TrimPattern(const Pattern& pattern) const {
    if (pattern.empty() || pattern[0].empty()) {
        return pattern;
    }

    int height = static_cast<int>(pattern.size());
    int width = static_cast<int>(pattern[0].size());

    // Находим границы непустой области
    int minX = width, maxX = -1;
    int minY = height, maxY = -1;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (pattern[y][x]) {
                minX = findMin(minX, x);
                maxX = findMax(maxX, x);
                minY = findMin(minY, y);
                maxY = findMax(maxY, y);
            }
        }
    }

    // Если паттерн пустой, возвращаем пустой Pattern
    if (maxX < 0 || maxY < 0) {
        return Pattern();
    }

    // Создаём обрезанный паттерн
    int newWidth = maxX - minX + 1;
    int newHeight = maxY - minY + 1;
    Pattern trimmedPattern(newHeight, std::vector<bool>(newWidth, false));

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            trimmedPattern[y][x] = pattern[y + minY][x + minX];
        }
    }

    return trimmedPattern;
}

//void PatternManager::loadPatternList(const std::string& patternFolder) {
//    patternList.clear(); // Очищаем существующий список
//
//    try {
//        // Проверяем, существует ли папка
//        if (!std::filesystem::exists(patternFolder)) {
//            std::cerr << "Папка " << patternFolder << " не существует!" << std::endl;
//            return;
//        }
//
//        // Перебираем все файлы в папке
//        for (const auto& entry : std::filesystem::directory_iterator(patternFolder)) {
//            if (entry.path().extension() == ".cells") {
//                patternList.push_back(entry.path().string());
//            }
//        }
//
//        // Сортируем список для удобства (опционально)
//        std::sort(patternList.begin(), patternList.end());
//
//        std::cout << "Найдено " << patternList.size() << " файлов .cells в папке " << patternFolder << std::endl;
//    }
//    catch (const std::filesystem::filesystem_error& e) {
//        std::cerr << "Ошибка при сканировании папки: " << e.what() << std::endl;
//    }
//}