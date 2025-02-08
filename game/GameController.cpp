#include "GameController.h"

GameController::GameController(int width, int height, float cellSize)
    : grid(width, height), gameOfLife(grid), cellSize(cellSize), isRunning(false), showGrid(true), currentPatternRotator(0){
    currentPattern = glider;
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // »нициализаци€ генератора случайных чисел
}

void GameController::randomizeGrid() {
    if (isRunning) return;
    std::random_device rd;  // “олько дл€ инициализации генератора
    std::mt19937 gen(rd()); // —тандартный мерсенновский твистер
    std::uniform_int_distribution<> disGliders(3, 15); // √енераци€ количества глайдеров
    std::uniform_int_distribution<> disX(0, grid.getWidth() - 3); // √енераци€ X-координаты
    std::uniform_int_distribution<> disY(0, grid.getHeight() - 3); // √енераци€ Y-координаты
    std::uniform_int_distribution<> disTipe(0, 3); // √енераци€ чисел от 0 до 3

    int numberOfGliders = disGliders(gen); // —лучайное количество глайдеров

    for (auto i = 0; i < numberOfGliders; ++i) {
        int startX = disX(gen);
        int startY = disY(gen);
        // –азмещаем фигуру на сетке в случайном месте
        placePattern(startX, startY, gosperGliderGun);
    }
}

void GameController::placePattern(int startX, int startY, const Pattern& pattern) {
    if (isRunning) return;
    int patternHeight = pattern.size();
    int patternWidth = pattern[0].size();

    // ѕровер€ем, помещаетс€ ли фигура в пределах сетки
    if (startX + patternWidth <= grid.getWidth() && startY + patternHeight <= grid.getHeight()) {
        for (auto y = patternHeight - 1; y >= 0; --y) { // »терируемс€ снизу вверх
            for (auto x = 0; x < patternWidth; ++x) {
                // »нвертируем x, чтобы перевернуть паттерн по горизонтали
                grid.setCellState(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), pattern[y][x]);
                if (pattern[y][x]) {
                    grid.getCell(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y)).setColor(Vector3d(0.5f, 0.9f, 0.5f));
                    gameOfLife.SetCellColor(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), Vector3d(0.5f, 0.9f, 0.5f));
                }
                else {
                    grid.getCell(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y)).setColor(Vector3d(0.1f, 0.1f, 0.1f));
                    gameOfLife.SetCellColor(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), Vector3d(0.1f, 0.1f, 0.1f));
                }
                
            }
        }
    }
}


void GameController::randomizeGrid(float density) {
    if (isRunning) return;
    // √енератор случайных чисел
    std::random_device rd;  // “олько дл€ инициализации генератора
    std::mt19937 gen(rd()); // —тандартный мерсенновский твистер
    std::uniform_real_distribution<> dis(0.0, 1.0); // –авномерное распределение

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            // ≈сли случайное число меньше density, клетка становитс€ живой
            if (dis(gen) < density) {
                grid.setCellState(x, y, true);
                grid.getCell(x, y).setColor(Vector3d(0.0f, 0.6f, 0.0f));
                gameOfLife.SetCellColor(x, y, Vector3d(0.0f, 0.6f, 0.0f));
            }
            else {
                grid.setCellState(x, y, false);
                grid.getCell(x, y).setColor(Vector3d(0.0f, 0.0f, 0.0f));
                gameOfLife.SetCellColor(x, y, Vector3d(0.0f, 0.0f, 0.0f));
            }
            //float r = static_cast<float>(rand()) / RAND_MAX;
            //float g = static_cast<float>(rand()) / RAND_MAX;
            //float b = static_cast<float>(rand()) / RAND_MAX;
            //grid.getCell(x, y).setColor(Vector3d(0.0f, 0.6f, 0.0f));
        }
    }
}

void GameController::clearGrid() {
    if (isRunning) return;
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            grid.setCellState(x, y, false); // ”станавливаем каждую клетку в мертвое состо€ние
            grid.getCell(x, y).setColor(Vector3d(0.0f, 0.0f, 0.0f));
            gameOfLife.SetCellColor(x, y, Vector3d(0.0f, 0.0f, 0.0f));
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
    if (isRunning) return;
    gameOfLife.nextGeneration();
}

void GameController::previousGeneration() {
    if (isRunning) return;
    gameOfLife.previousGeneration();
}

bool GameController::isSimulationRunning() const {
    return isRunning;
}

void GameController::toggleCellState(int x, int y) {
    if (isRunning) return;
    bool currentState = grid.getCellState(x, y);
    grid.setCellState(x, y, !currentState);
    if (!currentState) {
        //grid.getCell(x, y).setColor(Vector3d(0.1f, 0.4f, 0.1f));
        gameOfLife.SetCellColor(x, y, Vector3d(0.1f, 0.4f, 0.1f));
    }
    else {
        //grid.getCell(x, y).setColor(Vector3d(0.0f, 0.0f, 0.0f));
        gameOfLife.SetCellColor(x, y, Vector3d(0.0f, 0.0f, 0.0f));
    }
    
}

bool GameController::getCellState(int x, int y) const {
    return grid.getCellState(x, y);
}

void GameController::resizeGrid(int newWidth, int newHeight) {
    // ≈сли симул€ци€ работает, сбросим ее, чтобы избежать некорректной работы с новыми размерами
    if (isRunning) {
        stopSimulation();
    }
    if (newWidth <= 0 || newHeight <= 0) return; // ѕроверка на положительный размер

    // —оздаем новую сетку с новыми размерами
    Grid newGrid(newWidth, newHeight);

    //  опируем старые данные в новую сетку, где это возможно
    for (int y = 0; y < grid.getHeight(); ++y) {
        if (y >= newHeight) break; // ¬ыходим из цикла, если превышаем новую высоту
        for (int x = 0; x < grid.getWidth(); ++x) {
            if (x >= newWidth) break; // ¬ыходим из цикла, если превышаем новую ширину
            newGrid.setCellState(x, y, grid.getCellState(x, y));
        }
    }

    // ќбновл€ем текущую сетку
    grid = std::move(newGrid);

    // ќбновл€ем ссылку на сетку в GameOfLife
    gameOfLife.updateGridReference(grid);
}

void GameController::setCurrentPattern(int patternNumber) {
    switch (patternNumber) {
    case 1: 
        currentPattern = glider;
        break;
    case 2: 
        currentPattern = blinker; 
        break;
    case 3: 
        currentPattern = toad; 
        break;
    case 4: 
        currentPattern = beacon; 
        break;
    case 5: 
        currentPattern = pentadecathlon; 
        break;
    case 6: 
        currentPattern = gosperGliderGun;
        break;
    default:
        break;
    }
}

void GameController::setCurrentPatternRotator(int patternRotator) {
     currentPatternRotator = patternRotator;
}

void GameController::PlacePattern(int startX, int startY) {
    if (isRunning) return;
    
    static const Rotation rotationByIndex[] = {
        Rotation::Rotate90,
        Rotation::Rotate180,
        Rotation::Rotate270,
        Rotation::FlipHorizontal,
        Rotation::FlipVertical
    };

    Pattern newPattern = currentPattern;

    if (currentPatternRotator > 0) { // наш комбобокс имеет 6 значений. а ротатор всего 5. первый индекс это отсутствие вращени€
        if (currentPatternRotator >= 1 && currentPatternRotator < 6) { // ѕроверка на допустимость индекса
            Rotation rotation = rotationByIndex[currentPatternRotator - 1]; // текущий индекс вращение - индекс отсутстви€ вращени€
            newPattern = rotateOrFlip(currentPattern, rotation);
        }
    }

    if (!currentPattern.empty()) {
        placePattern(startX, startY, newPattern); // »спользуем уже существующий метод
    }
}

void GameController::setSimulationSpeed(float speed) {
    // speed - это количество секунд реального времени между обновлени€ми. ћеньше значение - быстрее симул€ци€.
    simulationSpeed = speed;
}

Pattern GameController::rotateOrFlip(const Pattern& pattern, Rotation rotation) {
    int rows = pattern.size();
    int cols = pattern[0].size();
    Pattern result;

    switch (rotation) {
    case Rotation::Rotate90:
        result = Pattern(cols, std::vector<bool>(rows, false));
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result[j][rows - 1 - i] = pattern[i][j];
            }
        }
        break;

    case Rotation::Rotate180:
        result = Pattern(rows, std::vector<bool>(cols, false));
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result[rows - 1 - i][cols - 1 - j] = pattern[i][j];
            }
        }
        break;

    case Rotation::Rotate270:
        result = Pattern(cols, std::vector<bool>(rows, false));
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result[cols - 1 - j][i] = pattern[i][j];
            }
        }
        break;

    case Rotation::FlipHorizontal:
        result = Pattern(rows, std::vector<bool>(cols, false));
        for (int i = 0; i < rows; ++i) {
            std::reverse_copy(pattern[i].begin(), pattern[i].end(), result[i].begin());
        }
        break;

    case Rotation::FlipVertical:
        result = Pattern(rows, std::vector<bool>(cols, false));
        for (int i = 0; i < rows; ++i) {
            result[rows - 1 - i] = pattern[i];
        }
        break;
    }

    return result;
}


