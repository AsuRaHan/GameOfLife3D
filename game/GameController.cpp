#include "GameController.h"

GameController::GameController(int width, int height, float cellSize)
    : grid(width, height), gameOfLife(grid), cellSize(cellSize), isRunning(false), showGrid(true) {
    currentPattern = glider;
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Инициализация генератора случайных чисел
}

void GameController::randomizeGrid() {

    std::random_device rd;  // Только для инициализации генератора
    std::mt19937 gen(rd()); // Стандартный мерсенновский твистер
    std::uniform_int_distribution<> disGliders(3, 15); // Генерация количества глайдеров
    std::uniform_int_distribution<> disX(0, grid.getWidth() - 3); // Генерация X-координаты
    std::uniform_int_distribution<> disY(0, grid.getHeight() - 3); // Генерация Y-координаты
    std::uniform_int_distribution<> disTipe(0, 3); // Генерация чисел от 0 до 3

    int numberOfGliders = disGliders(gen); // Случайное количество глайдеров

    for (auto i = 0; i < numberOfGliders; ++i) {
        int startX = disX(gen);
        int startY = disY(gen);
        // Размещаем глайдер на сетке

        int randomCondition = disTipe(gen); // Генерируем случайное число

        switch (randomCondition) {
        case 0:
            placePattern(startX, startY, gosperGliderGun);
            break;
        case 1:
            placePattern(startX, startY, gosperGliderGunFlipped);
            break;
        case 2:
            placePattern(startX, startY, gosperGliderGunVertical);
            break;
        case 3:
            placePattern(startX, startY, gosperGliderGunVerticalFlipped);
            break;
        default:
            // Это условие не должно срабатывать, так как мы ограничены диапазоном от 0 до 3
            break;
        }
        
    }
}

void GameController::placePattern(int startX, int startY, const Pattern& pattern) {
    int patternHeight = pattern.size();
    int patternWidth = pattern[0].size();

    // Проверяем, помещается ли фигура в пределах сетки
    if (startX + patternWidth <= grid.getWidth() && startY + patternHeight <= grid.getHeight()) {
        for (auto y = patternHeight - 1; y >= 0; --y) { // Итерируемся снизу вверх
            for (auto x = 0; x < patternWidth; ++x) {
                // Инвертируем x, чтобы перевернуть паттерн по горизонтали
                grid.setCellState(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), pattern[y][x]);
                if (pattern[y][x]) {
                    grid.getCell(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y)).setColor(Vector3d(0.5f, 0.9f, 0.5f));
                }
                else {
                    grid.getCell(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y)).setColor(Vector3d(0.1f, 0.1f, 0.1f));
                }
                
            }
        }
    }
}


void GameController::randomizeGrid(float density) {
    // Генератор случайных чисел
    std::random_device rd;  // Только для инициализации генератора
    std::mt19937 gen(rd()); // Стандартный мерсенновский твистер
    std::uniform_real_distribution<> dis(0.0, 1.0); // Равномерное распределение

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            // Если случайное число меньше density, клетка становится живой
            if (dis(gen) < density) {
                grid.setCellState(x, y, true);
                grid.getCell(x, y).setColor(Vector3d(0.0f, 0.6f, 0.0f));
            }
            else {
                grid.setCellState(x, y, false);
                grid.getCell(x, y).setColor(Vector3d(0.0f, 0.0f, 0.0f));
            }
            //float r = static_cast<float>(rand()) / RAND_MAX;
            //float g = static_cast<float>(rand()) / RAND_MAX;
            //float b = static_cast<float>(rand()) / RAND_MAX;
            //grid.getCell(x, y).setColor(Vector3d(0.0f, 0.6f, 0.0f));
        }
    }
}

void GameController::clearGrid() {
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            grid.setCellState(x, y, false); // Устанавливаем каждую клетку в мертвое состояние
            grid.getCell(x, y).setColor(Vector3d(0.0f, 0.0f, 0.0f));
        }
    }
}

void GameController::update(float deltaTime) {
    if (isRunning) {
        //frameTimeAccumulator += deltaTime;
        //if (frameTimeAccumulator >= simulationSpeed) {
            gameOfLife.nextGeneration();
        //    frameTimeAccumulator -= simulationSpeed;
        //}
    }
}

void GameController::startSimulation() {
    isRunning = true;
}

void GameController::stopSimulation() {
    isRunning = false;
}

void GameController::stepSimulation() {
    if (!isRunning) {
        gameOfLife.nextGeneration();
    }
}

void GameController::previousGeneration() {
    if (!isRunning) {
        gameOfLife.previousGeneration();
    }
}
bool GameController::isSimulationRunning() const {
    return isRunning;
}

void GameController::toggleCellState(int x, int y) {
    bool currentState = grid.getCellState(x, y);
    grid.setCellState(x, y, !currentState);
    if (!currentState) {
        grid.getCell(x, y).setColor(Vector3d(0.1f, 0.4f, 0.1f));
    }
    else {
        grid.getCell(x, y).setColor(Vector3d(0.0f, 0.0f, 0.0f));
    }
    
}

bool GameController::getCellState(int x, int y) const {
    return grid.getCellState(x, y);
}

void GameController::resizeGrid(int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) return; // Проверка на положительный размер

    // Создаем новую сетку с новыми размерами
    Grid newGrid(newWidth, newHeight);

    // Копируем старые данные в новую сетку, где это возможно
    for (int y = 0; y < grid.getHeight(); ++y) {
        if (y >= newHeight) break; // Выходим из цикла, если превышаем новую высоту
        for (int x = 0; x < grid.getWidth(); ++x) {
            if (x >= newWidth) break; // Выходим из цикла, если превышаем новую ширину
            newGrid.setCellState(x, y, grid.getCellState(x, y));
        }
    }

    // Обновляем текущую сетку
    grid = std::move(newGrid);

    // Обновляем ссылку на сетку в GameOfLife
    gameOfLife.updateGridReference(grid);

    // Если симуляция работает, сбросим ее, чтобы избежать некорректной работы с новыми размерами
    if (isRunning) {
        stopSimulation();
    }
}

void GameController::setCurrentPattern(int patternNumber) {
    switch (patternNumber) {
    case 1: currentPattern = glider; break;
    case 2: currentPattern = blinker; break;
    case 3: currentPattern = toad; break;
    case 4: currentPattern = beacon; break;
    case 5: currentPattern = pentadecathlon; break;
    case 6: currentPattern = gosperGliderGun; break;
    case 7: currentPattern = gosperGliderGunFlipped; break;
    case 8: currentPattern = gosperGliderGunVertical; break;
    case 9: currentPattern = gosperGliderGunVerticalFlipped; break;
    default:
        // Можно установить по умолчанию какой-то паттерн или оставить текущий без изменений
        currentPattern = glider;
        break;
    }
}

void GameController::PlacePattern(int startX, int startY) {
    if (!currentPattern.empty()) {
        placePattern(startX, startY, currentPattern); // Используем уже существующий метод
    }
}

void GameController::setSimulationSpeed(float speed) {
    // speed - это количество секунд реального времени между обновлениями. Меньше значение - быстрее симуляция.
    simulationSpeed = speed;
}