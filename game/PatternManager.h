#pragma once
#ifndef PATTERNMANAGER_H_
#define PATTERNMANAGER_H_

#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

class GameController;
using Pattern = std::vector<std::vector<bool>>;

class PatternManager {
public:
    // Конструктор
    PatternManager() = default;

    // Загружает паттерн из файла .cells и возвращает его в формате Pattern
    Pattern LoadPatternFromCells(const std::string& filename) const;

    // Метод для загрузки и немедленного размещения паттерна в игре
    void LoadAndPlacePattern(GameController& gameController, const std::string& filename, int startX, int startY) const;

    // Метод для проверки, может ли паттерн поместиться в сетку с заданными координатами
    bool CanPlacePattern(const GameController& gameController, const Pattern& pattern, int startX, int startY) const;

private:
    // Вспомогательный метод для обрезки пустых строк и столбцов (оптимизация)
    Pattern TrimPattern(const Pattern& pattern) const;
};

#endif // PATTERNMANAGER_H_