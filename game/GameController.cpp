#include "GameController.h"

GameController::GameController(int width, int height, float cellSize)
    : grid(width, height), 
    gpuAutomaton(grid.getWidth(), grid.getHeight()),
    cellSize(cellSize), 
    isRunning(false), showGrid(true), showUI(true), isWorldToroidal(true),
    currentPatternRotator(0), isSelectionActive(false),
    rendererProvider(nullptr)
{
    currentPattern = glider;
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Инициализация генератора случайных чисел
    grid.SetGPUAutomaton(&gpuAutomaton);
    gpuAutomaton.SetToroidal(isWorldToroidal);
}

void GameController::randomizeGrid() {
    if (isRunning) return;
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
        // Размещаем фигуру на сетке в случайном месте
        placePattern(startX, startY, gosperGliderGun);
    }
}

void GameController::placePattern(int startX, int startY, const Pattern& pattern) {
    if (isRunning) return;
    int patternHeight = static_cast<int>(pattern.size());
    int patternWidth = static_cast<int>(pattern[0].size());

    // Проверяем, помещается ли фигура в пределах сетки
    if (startX + patternWidth <= grid.getWidth() && startY + patternHeight <= grid.getHeight()) {
        for (auto y = patternHeight - 1; y >= 0; --y) { // Итерируемся снизу вверх
            for (auto x = 0; x < patternWidth; ++x) {
                // Инвертируем x, чтобы перевернуть паттерн по горизонтали
                grid.setCellState(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), pattern[y][x]);
                if (pattern[y][x]) {
                    grid.setCellColor(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), 0.5f, 0.9f, 0.5f);
                }
                else {
                    grid.setCellColor(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), 0.1f, 0.1f, 0.2f);
                }
            }
        }
    }
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

    if (currentPatternRotator > 0) { // наш комбобокс имеет 6 значений. а ротатор всего 5. первый индекс это отсутствие вращения
        if (currentPatternRotator >= 1 && currentPatternRotator < 6) { // Проверка на допустимость индекса
            Rotation rotation = rotationByIndex[currentPatternRotator - 1]; // текущий индекс вращение - индекс отсутствия вращения
            newPattern = rotateOrFlip(currentPattern, rotation);
        }
    }

    if (!currentPattern.empty()) {
        placePattern(startX, startY, newPattern); // Используем уже существующий метод
    }
}

void GameController::setWoldToroidal(bool wt)
{
    isWorldToroidal = wt;
    gpuAutomaton.SetToroidal(isWorldToroidal);
}

void GameController::randomizeGrid(float density) {
    if (isRunning) return;
    // Генератор случайных чисел
    std::random_device rd;  // Только для инициализации генератора
    std::mt19937 gen(rd()); // Стандартный мерсенновский твистер
    std::uniform_real_distribution<> dis(0.0, 1.0); // Равномерное распределение

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            // Если случайное число меньше density, клетка становится живой
            if (dis(gen) < density) {
                grid.setCellState(x, y, true);
                grid.setCellColor(x, y, 0.0f, 0.6f, 0.0f);
            }
            else {
                grid.setCellState(x, y, false);
                grid.setCellColor(x, y, 0.0f, 0.0f, 0.0f);
            }
        }
    }
}

void GameController::clearGrid() {
    if (isRunning) return;
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            grid.setCellState(x, y, false); // Устанавливаем каждую клетку в мертвое состояние
            grid.setCellColor(x, y, 0.0f, 0.0f, 0.0f);
        }
    }
}

void GameController::update(float deltaTime) {
    if (isRunning) {
        //frameTimeAccumulator += deltaTime;
        //if (frameTimeAccumulator >= simulationSpeed) {
        gpuAutomaton.Update();
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
    gpuAutomaton.Update();
}

bool GameController::isSimulationRunning() const {
    return isRunning;
}

void GameController::toggleCellState(int x, int y) {
    if (isRunning) return;
    bool currentState = grid.getCellState(x, y);
    grid.setCellState(x, y, !currentState);
    if (!currentState) {
        grid.setCellColor(x, y, 0.1f, 0.4f, 0.1f);
    }
    else {
        grid.setCellColor(x, y, 0.0f, 0.0f, 0.0f);
    }

}

void GameController::setLiveCell(int x, int y, bool state) {
    if (isRunning) return;
    grid.setCellState(x, y, state);
    grid.setCellColor(x, y, 0.1f, 0.4f, 0.1f);

}

bool GameController::getCellState(int x, int y) const {
    return grid.getCellState(x, y);
}

void GameController::SetCellInstanceProvider(IRendererProvider* provider) {
    rendererProvider = provider;
    //GameSimulation.SetCellProvider(provider); // Передаем провайдера в GameOfLife
}

void GameController::setFieldSize(int newWidth, int newHeight) {
    // Если симуляция работает, сбросим ее, чтобы избежать некорректной работы с новыми размерами
    if (isRunning) {
        stopSimulation();
    }
    if (newWidth <= 0 || newHeight <= 0) return; // Проверка на положительный размер

    grid.setSize(newWidth, newHeight);
    gpuAutomaton.SetNewGridSize(newWidth, newHeight);
    // Перед перестройкой буферов, уведомляем рендер о необходимости обновлений
    rendererProvider->RebuildGameField();
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

void GameController::setSimulationSpeed(float speed) {
    // speed - это количество секунд реального времени между обновлениями. Меньше значение - быстрее симуляция.
    simulationSpeed = speed;
}

Pattern GameController::rotateOrFlip(const Pattern& pattern, Rotation rotation) {
    int rows = static_cast<int>(pattern.size());
    int cols = static_cast<int>(pattern[0].size());
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

bool GameController::saveGameState(const std::string& filename) const {
    return GameStateManager::saveGameState(grid, filename);
}

bool GameController::loadGameState(const std::string& filename) {
    bool gameIsSave = GameStateManager::loadGameState(grid, filename);
    if (!gameIsSave) return gameIsSave;
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            if (grid.getCellState(x,y)) {
                grid.setCellColor(x, y, 0.0f, 0.6f, 0.0f);
            }
            else {
                grid.setCellColor(x, y, 0.0f, 0.0f, 0.0f);
            }

        }
    }
    return gameIsSave;
}

void GameController::loadPatternList(const std::string& patternFolder) {
    patternList.clear(); // Очищаем существующий список

    try {
        // Проверяем, существует ли папка
        if (!std::filesystem::exists(patternFolder)) {
            std::cerr << "Папка " << patternFolder << " не существует!" << std::endl;
            return;
        }

        // Перебираем все файлы в папке
        for (const auto& entry : std::filesystem::directory_iterator(patternFolder)) {
            if (entry.path().extension() == ".cells") {
                patternList.push_back(entry.path().string());
            }
        }

        // Сортируем список для удобства (опционально)
        std::sort(patternList.begin(), patternList.end());

        std::cout << "Найдено " << patternList.size() << " файлов .cells в папке " << patternFolder << std::endl;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Ошибка при сканировании папки: " << e.what() << std::endl;
    }
}

void GameController::setCurrentPatternFromFile(const std::string& filename, int startX, int startY) {
    PatternManager patternManager;
    try {
        currentPattern = patternManager.LoadPatternFromCells(filename);
        //patternManager.LoadAndPlacePattern(*this, filename, startX, startY);
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при выборе паттерна: " << e.what() << std::endl;
    }
}

void GameController::SetSelectionStart(int x, int y) {
    selectionStart = Vector3d(x, y, 0.0f); // Z-координату устанавливаем в 0, так как работаем в 2D
    selectionEnd = Vector3d(x, y, 0.0f); // Аналогично для конца выделения
    isSelectionActive = true;
}

void GameController::SetSelectionEnd(int x, int y) {
    selectionEnd = Vector3d(x, y, 0.0f); // Аналогично для конца выделения
    // Очищаем список выделенных клеток перед добавлением новых
    ClearSelectedCells();

    int startX = static_cast<int>(selectionStart.X());
    int startY = static_cast<int>(selectionStart.Y());
    int endX = static_cast<int>(selectionEnd.X());
    int endY = static_cast<int>(selectionEnd.Y());

    // Определяем минимальные и максимальные координаты для правильного выделения
    int minX = (startX < endX) ? startX : endX;
    int maxX = (startX > endX) ? startX : endX;
    int minY = (startY < endY) ? startY : endY;
    int maxY = (startY > endY) ? startY : endY;

    // Добавляем все клетки в пределах выделенного прямоугольника
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            if (x >= 0 && y >= 0 && x < grid.getWidth() && y < grid.getHeight()) { // Проверка границ сетки
                AddSelectedCell(x, y);
            }
        }
    }
}

void GameController::AddSelectedCell(int x, int y) {
    selectedCells.push_back(Vector3d(x, y, 0.0f)); // Добавляем клетку в список выделенных, z всегда 0 в 2D
}

void GameController::ClearSelectedCells() {
    selectedCells.clear(); // Очищаем список выделенных клеток
}

void GameController::KillSelectedCells() {
    for (const auto& cell : selectedCells) {
        int x = static_cast<int>(cell.X());
        int y = static_cast<int>(cell.Y());
        grid.setCellState(x, y, false);
        grid.setCellColor(x, y, 0.0f, 0.0f, 0.0f); // Устанавливаем цвет для мертвой клетки
    }
    isSelectionActive = false;
}

void GameController::ReviveSelectedCells() {
    for (const auto& cell : selectedCells) {
        int x = static_cast<int>(cell.X());
        int y = static_cast<int>(cell.Y());
        grid.setCellState(x, y, true);
        grid.setCellColor(x, y, 0.1f, 0.4f, 0.1f); // Устанавливаем цвет для живой клетки
    }
    isSelectionActive = false;
}