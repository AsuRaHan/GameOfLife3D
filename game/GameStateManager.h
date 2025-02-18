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
#include <cstring> // ��� memcmp

#pragma comment(lib, "Cabinet.lib")

#define GAME_FILE_MAGIC_NUMBER "GOLB"

class GameStateManager {
public:
    // ������� ����������/�������� (������ ��������� ������)
    static bool saveGameState(const Grid& grid, const std::string& filename);
    static bool loadGameState(Grid& grid, const std::string& filename);

    static bool loadBinaryGameState(Grid& grid, const std::string& filename);
    static bool saveBinaryGameState(const Grid& grid, const std::string& filename);
private:
    struct SectionHeader {
        char type[4]; // 4 ������� ��� ���� (��������, "STAT" ��� ���������, "COLR" ��� ������)
        SIZE_T size;  // ������ ������ � ������
    };
    // ��������������� ������
    static bool validateDimensions(const Grid& grid, int width, int height);

    static bool CompressData(const BYTE* uncompressedData, SIZE_T uncompressedSize, BYTE** compressedData, SIZE_T* compressedSize);
    static bool DecompressData(const BYTE* compressedData, SIZE_T compressedSize, BYTE** decompressedData, SIZE_T* decompressedSize);
};

#endif // GAMESTATEMANAGER_H_ 