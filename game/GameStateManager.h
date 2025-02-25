#pragma once
#ifndef GAMESTATEMANAGER_H_
#define GAMESTATEMANAGER_H_


#include "Grid.h"
#include <windows.h>
#include <compressapi.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring> // для memcmp
#include <filesystem>

#pragma comment(lib, "Cabinet.lib")

#define GAME_FILE_MAGIC_NUMBER "GOLB"

class GameStateManager {
public:
    // Базовое сохранение/загрузка
    static bool loadBinaryGameState(Grid& grid, const std::string& filename, int& loadStatus);
    static bool saveBinaryGameState(const Grid& grid, const std::string& filename);

    // сохранение выделенной области игрового поля как петтерна
    static bool SavePatternToCellsFile(const std::string& filename, const std::vector<std::vector<bool>>& pattern, const std::string& patternName);
private:
    struct SectionHeader {
        char type[4]; // 4 символа для типа ("STAT" для состояний, "COLR" для цветов)
        SIZE_T size;  // Размер данных в байтах
    };
    // Вспомогательные методы

    static bool CompressData(const BYTE* uncompressedData, SIZE_T uncompressedSize, BYTE** compressedData, SIZE_T* compressedSize);
    static bool DecompressData(const BYTE* compressedData, SIZE_T compressedSize, BYTE** decompressedData, SIZE_T* decompressedSize);
};

#endif // GAMESTATEMANAGER_H_ 