#pragma once
#ifndef GAMESTATEMANAGER_H_
#define GAMESTATEMANAGER_H_

#include <string>
#include "Grid.h"

class GameStateManager {
public:
    // Ѕазовое сохранение/загрузка (только состо€ни€ клеток)
    static bool saveGameState(const Grid& grid, const std::string& filename);
    static bool loadGameState(Grid& grid, const std::string& filename);

    // –асширенное сохранение/загрузка (с дополнительными параметрами)
    static bool saveGameStateCSV(const Grid& grid, const std::string& filename);
    static bool loadGameStateCSV(Grid& grid, const std::string& filename);

private:
    // ¬спомогательные методы
    static bool validateFile(const std::string& filename);
    static bool validateDimensions(const Grid& grid, int width, int height);
};

#endif // GAMESTATEMANAGER_H_ 