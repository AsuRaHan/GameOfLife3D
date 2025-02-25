#include "GameStateManager.h"


bool GameStateManager::SavePatternToCellsFile(const std::string& filename, const std::vector<std::vector<bool>>& pattern, const std::string& patternName) {
    if (pattern.empty() || pattern[0].empty()) return false;

    std::ofstream file("./patterns/"+filename);
    if (!file.is_open()) {
        return false; // Ошибка открытия файла
    }

    // Записываем метаданные
    file << "!Name: " << patternName << "\n";
    file << "!Created by Game of Life 3D\n";
    file << "!Welcom to https://github.com/AsuRaHan/GameOfLife3D\n";

    // Записываем паттерн
    for (size_t y = 0; y < pattern.size(); ++y) {
        for (size_t x = 0; x < pattern[y].size(); ++x) {
            file << (pattern[y][x] ? "O" : ".");
        }
        file << "\n";
    }

    file.close();
    return true;
}

bool GameStateManager::CompressData(const BYTE* uncompressedData, SIZE_T uncompressedSize, BYTE** compressedData, SIZE_T* compressedSize) {
    COMPRESSOR_HANDLE compressor;
    if (!CreateCompressor(COMPRESS_ALGORITHM_XPRESS, NULL, &compressor)) {
        // Обработка ошибки
        std::cerr << "Failed to create compressor: " << GetLastError() << std::endl;
        return false;
    }

    SIZE_T estimatedSize = 0;
    if (!Compress(compressor, uncompressedData, uncompressedSize, NULL, 0, &estimatedSize)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            *compressedData = (BYTE*)malloc(estimatedSize);
            if (*compressedData == NULL) {
                // Обработка ошибки выделения памяти
                std::cerr << "Memory allocation failed for compression buffer" << std::endl;
                CloseCompressor(compressor);
                return false;
            }
            if (!Compress(compressor, uncompressedData, uncompressedSize, *compressedData, estimatedSize, (SIZE_T*)compressedSize)) {
                // Обработка ошибки
                std::cerr << "Compression failed: " << GetLastError() << std::endl;
                free(*compressedData);
                *compressedData = NULL;
                CloseCompressor(compressor);
                return false;
            }
        }
        else {
            // Обработка других ошибок компрессии
            std::cerr << "Unexpected error during compression size estimation: " << GetLastError() << std::endl;
            CloseCompressor(compressor);
            return false;
        }
    }

    CloseCompressor(compressor);
    return true;
}

bool GameStateManager::DecompressData(const BYTE* compressedData, SIZE_T compressedSize, BYTE** decompressedData, SIZE_T* decompressedSize) {
    DECOMPRESSOR_HANDLE decompressor;
    if (!CreateDecompressor(COMPRESS_ALGORITHM_XPRESS, NULL, &decompressor)) {
        // Обработка ошибки
        std::cerr << "Failed to create decompressor: " << GetLastError() << std::endl;
        return false;
    }

    SIZE_T neededSize = 0;
    if (!Decompress(decompressor, compressedData, compressedSize, NULL, 0, &neededSize)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            *decompressedData = (BYTE*)malloc(neededSize);
            if (*decompressedData == NULL) {
                // Обработка ошибки выделения памяти
                std::cerr << "Memory allocation failed for decompression buffer" << std::endl;
                CloseDecompressor(decompressor);
                return false;
            }
            if (!Decompress(decompressor, compressedData, compressedSize, *decompressedData, neededSize, (SIZE_T*)decompressedSize)) {
                // Обработка ошибки
                std::cerr << "Decompression failed: " << GetLastError() << std::endl;
                free(*decompressedData);
                *decompressedData = NULL;
                CloseDecompressor(decompressor);
                return false;
            }
        }
        else {
            // Обработка других ошибок декомпрессии
            std::cerr << "Unexpected error during decompression size estimation: " << GetLastError() << std::endl;
            CloseDecompressor(decompressor);
            return false;
        }
    }

    CloseDecompressor(decompressor);
    return true;
}


bool GameStateManager::saveBinaryGameState(const Grid& grid, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл для бинарной записи: " << filename << std::endl;
        return false;
    }

    auto* automaton = grid.GetGPUAutomaton();
    int width = grid.getWidth();
    int height = grid.getHeight();

    // Сохраняем сигнатуру
    file.write(GAME_FILE_MAGIC_NUMBER, strlen(GAME_FILE_MAGIC_NUMBER));

    // Записываем ширину и высоту
    file.write(reinterpret_cast<const char*>(&width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&height), sizeof(int));

    // Секция для состояний клеток
    std::vector<int> states;
    automaton->GetGridState(states);
    BYTE* compressedStates = NULL;
    SIZE_T compressedStatesSize = 0;
    if (CompressData(reinterpret_cast<BYTE*>(states.data()), states.size() * sizeof(int), &compressedStates, &compressedStatesSize)) {
        SectionHeader statesHeader = { {'S', 'T', 'A', 'T'}, compressedStatesSize };
        file.write(reinterpret_cast<const char*>(&statesHeader), sizeof(SectionHeader));
        file.write(reinterpret_cast<const char*>(compressedStates), compressedStatesSize);
        free(compressedStates);
    }
    else {
        std::cerr << "Ошибка при сжатии данных состояния клеток!" << std::endl;
        return false;
    }

    // Секция для цветов
    std::vector<float> colors;
    automaton->GetColorsBuf(colors);
    BYTE* compressedColors = NULL;
    SIZE_T compressedColorsSize = 0;
    if (CompressData(reinterpret_cast<BYTE*>(colors.data()), colors.size() * sizeof(float), &compressedColors, &compressedColorsSize)) {
        SectionHeader colorsHeader = { {'C', 'O', 'L', 'R'}, compressedColorsSize };
        file.write(reinterpret_cast<const char*>(&colorsHeader), sizeof(SectionHeader));
        file.write(reinterpret_cast<const char*>(compressedColors), compressedColorsSize);
        free(compressedColors);
    }
    else {
        std::cerr << "Ошибка при сжатии данных цветов!" << std::endl;
        return false;
    }

    return true;
}

bool GameStateManager::loadBinaryGameState(Grid& grid, const std::string& filename, int & loadStatus) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл для бинарного чтения: " << filename << std::endl;
        return false;
    }

    // Проверка сигнатуры
    char magic[5] = { 0 };
    file.read(magic, strlen(GAME_FILE_MAGIC_NUMBER));
    if (std::memcmp(magic, GAME_FILE_MAGIC_NUMBER, strlen(GAME_FILE_MAGIC_NUMBER)) != 0) {
        std::cerr << "Файл имеет неверный формат или поврежден." << std::endl;
        return false;
    }

    int width, height;
    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));

    // Проверка соответствия размеров
    auto* automaton = grid.GetGPUAutomaton();
    if (width != grid.getWidth() || height != grid.getHeight()) {
        std::cerr << "Размеры в загружаемом файле не совпадают с текущими размерами сетки. мир будет перестроен с новыми размерами. width=" << width << " height=" << height << std::endl;
        grid.setSize(width, height);
        automaton->SetNewGridSize(width, height);
        loadStatus = 2;
    }

    while (file) {
        SectionHeader header;
        if (!file.read(reinterpret_cast<char*>(&header), sizeof(SectionHeader))) break;

        std::vector<BYTE> compressedData(header.size);
        file.read(reinterpret_cast<char*>(compressedData.data()), header.size);

        if (strncmp(header.type, "STAT", 4) == 0) {
            BYTE* decompressedStates = NULL;
            SIZE_T decompressedStatesSize = width * height * sizeof(int);
            if (DecompressData(compressedData.data(), compressedData.size(), &decompressedStates, &decompressedStatesSize)) {
                std::vector<int> states(reinterpret_cast<int*>(decompressedStates), reinterpret_cast<int*>(decompressedStates) + decompressedStatesSize / sizeof(int));
                automaton->SetGridState(states);
                free(decompressedStates);
            }
            else {
                std::cerr << "Ошибка при распаковке данных состояния клеток!" << std::endl;
                loadStatus = 0;
                return false;
            }
        }
        else if (strncmp(header.type, "COLR", 4) == 0) {
            BYTE* decompressedColors = NULL;
            SIZE_T decompressedColorsSize = width * height * 4 * sizeof(float);
            if (DecompressData(compressedData.data(), compressedData.size(), &decompressedColors, &decompressedColorsSize)) {
                std::vector<float> colors(reinterpret_cast<float*>(decompressedColors), reinterpret_cast<float*>(decompressedColors) + decompressedColorsSize / sizeof(float));
                automaton->SetColorsBuf(colors);
                free(decompressedColors);
            }
            else {
                std::cerr << "Ошибка при распаковке данных цветов!" << std::endl;
                loadStatus = 0;
                return false;
            }
        }
    }
    return true;
}