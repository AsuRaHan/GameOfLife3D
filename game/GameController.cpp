#include "GameController.h"


GameController::GameController(int width, int height, float cellSize)
    : grid(width, height),
    cellSize(cellSize),
    isRunning(false), showGrid(true), showUI(true), isWorldToroidal(true),
    isPatternPlacementMode(false), isTurboBoost(false), currentPatternRotator(0),
    isSelectionActive(false), rendererProvider(nullptr) {
    //gameAutomaton = new GPUAutomaton(grid.getWidth(), grid.getHeight()); // Пока используем GPUAutomaton
    // gameAutomaton = new ModSystemAutomaton(grid.getWidth(), grid.getHeight());
    
    if (!ModManager::checkMods()) {
        std::cerr << "Error: Mods check failed. Using default." << std::endl;
        gameAutomaton = new GPUAutomaton(grid.getWidth(), grid.getHeight());
    }
    else {
        gameAutomaton = new ModSystemAutomaton(grid.getWidth(), grid.getHeight());
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    grid.SetGPUAutomaton(gameAutomaton);
    gameAutomaton->SetToroidal(isWorldToroidal);

    frameTimeAccumulator = GetTickCount64();
    isRunning = false;
}

GameController::~GameController() {
    delete gameAutomaton; // Очищаем память
}

void GameController::placePattern(int startX, int startY, const Pattern& pattern) {
    if (isRunning) return;
    int patternHeight = static_cast<int>(pattern.size());
    int patternWidth = static_cast<int>(pattern[0].size());

    if (startX < 0 || startY < 0 ||
        startX + patternWidth > grid.getWidth() ||
        startY + patternHeight > grid.getHeight()) {
        return; // Паттерн не помещается
    }

    // Проверяем, помещается ли фигура в пределах сетки
    if (startX + patternWidth <= grid.getWidth() && startY + patternHeight <= grid.getHeight()) {
        for (auto y = patternHeight - 1; y >= 0; --y) { // Итерируемся снизу вверх
            for (auto x = 0; x < patternWidth; ++x) {
                // Инвертируем x, чтобы перевернуть паттерн по горизонтали
                if (pattern[y][x]) {
                    grid.SetCellType(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), cellType);
                }
                else {
                    grid.SetCellType(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), -1);
                    grid.setCellColor(startX + (patternWidth - 1 - x), startY + (patternHeight - 1 - y), 0.1f, 0.1f, 0.2f);
                }
            }
        }
        SetPatternPlacementMode(false);
    }
}

void GameController::PlacePattern(int startX, int startY) {
    if (isRunning) return;

    if (!insertablePattern.empty()) {
        placePattern(startX, startY, insertablePattern); // Используем уже существующий метод
    }
}

void GameController::setWoldToroidal(bool wt)
{
    isWorldToroidal = wt;
    gameAutomaton->SetToroidal(isWorldToroidal);
}

void GameController::randomizeGrid(float density) {
    if (isRunning) return;
        // Генерация случайного seed
    std::random_device rd;
    std::mt19937 gen(rd());
    unsigned int seed = gen(); // Генерируем случайное число для seed
    // Вызов метода RandomizeGrid с density и seed
    gameAutomaton->RandomizeGrid(density, seed);
}

void GameController::clearGrid() {
    if (isRunning) return;
    gameAutomaton->ClearGrid();
}

void GameController::update() {
    static auto lastUpdate = std::chrono::steady_clock::now();
    if (isRunning) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastUpdate).count();

        if (elapsed >= simulationSpeed) {
            gameAutomaton->Update();
            lastUpdate = now;
            PerformanceStats::getInstance().recordSimulation();
        }

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
    gameAutomaton->Update();
}

bool GameController::isSimulationRunning() const {
    return isRunning;
}

void GameController::toggleCellState(int x, int y) {
    if (isRunning || isPatternPlacementMode) return;
    int currentState = grid.getCellState(x, y);
    if (currentState <= 0) {
        grid.SetCellType(x, y, cellType);
    }
    else {
        grid.SetCellType(x, y, -1);
    }

}

void GameController::setLiveCell(int x, int y, bool state) {
    if (isRunning || isPatternPlacementMode) return;
    grid.SetCellType(x, y, cellType);
}

bool GameController::getCellState(int x, int y) const {
    return grid.getCellState(x, y);
}

void GameController::SetRendererProvider(IRendererProvider* provider) {
    rendererProvider = provider;
}

void GameController::setFieldSize(int newWidth, int newHeight) {
    // Если симуляция работает, сбросим ее, чтобы избежать некорректной работы с новыми размерами
    if (isRunning) {
        stopSimulation();
    }
    if (newWidth <= 0 || newHeight <= 0) return; // Проверка на положительный размер

    grid.setSize(newWidth, newHeight);
    gameAutomaton->SetNewGridSize(newWidth, newHeight);
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
    insertablePattern = currentPattern;
    SetPatternPlacementMode(true);
}

void GameController::setCurrentPatternRotator(int patternRotator) {
     currentPatternRotator = patternRotator;

     static const Rotation rotationByIndex[] = {
    Rotation::Rotate90,
    Rotation::Rotate180,
    Rotation::Rotate270,
    Rotation::FlipHorizontal,
    Rotation::FlipVertical
     };

     insertablePattern = currentPattern;

     if (currentPatternRotator > 0) { // наш комбобокс имеет 6 значений. а ротатор всего 5. первый индекс это отсутствие вращения
         if (currentPatternRotator >= 1 && currentPatternRotator < 6) { // Проверка на допустимость индекса
             Rotation rotation = rotationByIndex[currentPatternRotator - 1]; // текущий индекс вращение - индекс отсутствия вращения
             insertablePattern = rotateOrFlip(currentPattern, rotation);
         }
     }
     SetPatternPlacementMode(true);
}

void GameController::flipOrRotateInsertablePattern(int patternRotator) {
    currentPatternRotator = patternRotator;

    static const Rotation rotationByIndex[] = {
   Rotation::Rotate90,
   Rotation::Rotate180,
   Rotation::Rotate270,
   Rotation::FlipHorizontal,
   Rotation::FlipVertical
    };

    if (currentPatternRotator > 0) { // наш комбобокс имеет 6 значений. а ротатор всего 5. первый индекс это отсутствие вращения
        if (currentPatternRotator >= 1 && currentPatternRotator < 6) { // Проверка на допустимость индекса
            Rotation rotation = rotationByIndex[currentPatternRotator - 1]; // текущий индекс вращение - индекс отсутствия вращения
            insertablePattern = rotateOrFlip(insertablePattern, rotation);
        }
    }
    SetPatternPlacementMode(true);
}

void GameController::setSimulationSpeed(int speed) {
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

bool GameController::saveGameState(const std::string& filename){
    return GameStateManager::saveBinaryGameState(grid, filename);
}

bool GameController::loadGameState(const std::string& filename) {
    int loadStatus = 0;
    bool retLoadVal = GameStateManager::loadBinaryGameState(grid, filename, loadStatus);
    if (retLoadVal) {
        if (loadStatus == 2) {
            rendererProvider->RebuildGameField();
        }
    }
    return retLoadVal;
}

void GameController::SaveSelectedPatternToFile(const std::string& filename, const std::string& patternName) {
    if (!isSelectionActive || selectedCells.empty()) return;

    // Сначала создаём паттерн из выделения
    InsertSelectedCellsAsPattern();

    // Сохраняем паттерн в файл
    if (!insertablePattern.empty()) {
        GameStateManager::SavePatternToCellsFile(filename, insertablePattern, patternName);
    }
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
        insertablePattern = patternManager.LoadPatternFromCells(filename);
        SetPatternPlacementMode(true);
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
        grid.SetCellType(x, y, -1);
    }
    isSelectionActive = false;
}

void GameController::ReviveSelectedCells() {
    for (const auto& cell : selectedCells) {
        int x = static_cast<int>(cell.X());
        int y = static_cast<int>(cell.Y());
        grid.SetCellType(x, y, cellType);
    }
    isSelectionActive = false;
}

void GameController::InsertSelectedCellsAsPattern() {
    if (!isSelectionActive) return;

    // Определяем границы выделения
    int minX = static_cast<int>(selectedCells[0].X()), maxX = minX, minY = static_cast<int>(selectedCells[0].Y()), maxY = minY;
    for (size_t i = 1; i < selectedCells.size(); ++i) {
        int x = static_cast<int>(selectedCells[i].X());
        int y = static_cast<int>(selectedCells[i].Y());

        // Обновляем минимальные значения
        if (x < minX) minX = x;
        if (y < minY) minY = y;

        // Обновляем максимальные значения
        if (x > maxX) maxX = x;
        if (y > maxY) maxY = y;
    }

    // Создаем паттерн
    Pattern newPattern(maxY - minY + 1, std::vector<bool>(maxX - minX + 1, false));
    // Заполняем паттерн, учитывая правильную ориентацию
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            // Инвертируем оси Y и X
            newPattern[maxY - y][maxX - x] = grid.getCellState(x, y);
        }
    }
    // Сохраняем паттерн
    currentPattern = newPattern;
    insertablePattern = currentPattern;
    isSelectionActive = false;
}

void GameController::SetCellType(int type)
{
    cellType = type;
}

void GameController::SetPatternPlacementMode(bool enabled)
{
    if (isRunning) return;

    if (!insertablePattern.empty()) {
        isPatternPlacementMode = enabled;
    }
    
}

void GameController::UpdatePatternPreviewPosition(int x, int y) {
    int gridWidth = grid.getWidth();
    int gridHeight = grid.getHeight();

    int patternWidth = currentPattern.empty() ? 0 : currentPattern[0].size();
    int patternHeight = currentPattern.empty() ? 0 : currentPattern.size();

    // Ограничиваем координаты, чтобы паттерн не выходил за пределы сетки
    if (x < 0) x = 0;
    else if (x + patternWidth > gridWidth) x = gridWidth - patternWidth;
    if (y < 0) y = 0;
    else if (y + patternHeight > gridHeight) y = gridHeight - patternHeight;

    patternPreviewPosition = Vector3d(x, y, 0.0f);
}