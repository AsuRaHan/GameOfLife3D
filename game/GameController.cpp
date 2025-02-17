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
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // ������������� ���������� ��������� �����
    grid.SetGPUAutomaton(&gpuAutomaton);
    gpuAutomaton.SetToroidal(isWorldToroidal);
}

void GameController::randomizeGrid() {
    if (isRunning) return;
    std::random_device rd;  // ������ ��� ������������� ����������
    std::mt19937 gen(rd()); // ����������� ������������� �������
    std::uniform_int_distribution<> disGliders(3, 15); // ��������� ���������� ���������
    std::uniform_int_distribution<> disX(0, grid.getWidth() - 3); // ��������� X-����������
    std::uniform_int_distribution<> disY(0, grid.getHeight() - 3); // ��������� Y-����������
    std::uniform_int_distribution<> disTipe(0, 3); // ��������� ����� �� 0 �� 3

    int numberOfGliders = disGliders(gen); // ��������� ���������� ���������

    for (auto i = 0; i < numberOfGliders; ++i) {
        int startX = disX(gen);
        int startY = disY(gen);
        // ��������� ������ �� ����� � ��������� �����
        placePattern(startX, startY, gosperGliderGun);
    }
}

void GameController::placePattern(int startX, int startY, const Pattern& pattern) {
    if (isRunning) return;
    int patternHeight = static_cast<int>(pattern.size());
    int patternWidth = static_cast<int>(pattern[0].size());

    // ���������, ���������� �� ������ � �������� �����
    if (startX + patternWidth <= grid.getWidth() && startY + patternHeight <= grid.getHeight()) {
        for (auto y = patternHeight - 1; y >= 0; --y) { // ����������� ����� �����
            for (auto x = 0; x < patternWidth; ++x) {
                // ����������� x, ����� ����������� ������� �� �����������
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

    if (currentPatternRotator > 0) { // ��� ��������� ����� 6 ��������. � ������� ����� 5. ������ ������ ��� ���������� ��������
        if (currentPatternRotator >= 1 && currentPatternRotator < 6) { // �������� �� ������������ �������
            Rotation rotation = rotationByIndex[currentPatternRotator - 1]; // ������� ������ �������� - ������ ���������� ��������
            newPattern = rotateOrFlip(currentPattern, rotation);
        }
    }

    if (!currentPattern.empty()) {
        placePattern(startX, startY, newPattern); // ���������� ��� ������������ �����
    }
}

void GameController::setWoldToroidal(bool wt)
{
    isWorldToroidal = wt;
    gpuAutomaton.SetToroidal(isWorldToroidal);
}

void GameController::randomizeGrid(float density) {
    if (isRunning) return;
    // ��������� ��������� �����
    std::random_device rd;  // ������ ��� ������������� ����������
    std::mt19937 gen(rd()); // ����������� ������������� �������
    std::uniform_real_distribution<> dis(0.0, 1.0); // ����������� �������������

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            // ���� ��������� ����� ������ density, ������ ���������� �����
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
            grid.setCellState(x, y, false); // ������������� ������ ������ � ������� ���������
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
    //GameSimulation.SetCellProvider(provider); // �������� ���������� � GameOfLife
}

void GameController::setFieldSize(int newWidth, int newHeight) {
    // ���� ��������� ��������, ������� ��, ����� �������� ������������ ������ � ������ ���������
    if (isRunning) {
        stopSimulation();
    }
    if (newWidth <= 0 || newHeight <= 0) return; // �������� �� ������������� ������

    grid.setSize(newWidth, newHeight);
    gpuAutomaton.SetNewGridSize(newWidth, newHeight);
    // ����� ������������ �������, ���������� ������ � ������������� ����������
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
    // speed - ��� ���������� ������ ��������� ������� ����� ������������. ������ �������� - ������� ���������.
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
    patternList.clear(); // ������� ������������ ������

    try {
        // ���������, ���������� �� �����
        if (!std::filesystem::exists(patternFolder)) {
            std::cerr << "����� " << patternFolder << " �� ����������!" << std::endl;
            return;
        }

        // ���������� ��� ����� � �����
        for (const auto& entry : std::filesystem::directory_iterator(patternFolder)) {
            if (entry.path().extension() == ".cells") {
                patternList.push_back(entry.path().string());
            }
        }

        // ��������� ������ ��� �������� (�����������)
        std::sort(patternList.begin(), patternList.end());

        std::cout << "������� " << patternList.size() << " ������ .cells � ����� " << patternFolder << std::endl;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "������ ��� ������������ �����: " << e.what() << std::endl;
    }
}

void GameController::setCurrentPatternFromFile(const std::string& filename, int startX, int startY) {
    PatternManager patternManager;
    try {
        currentPattern = patternManager.LoadPatternFromCells(filename);
        //patternManager.LoadAndPlacePattern(*this, filename, startX, startY);
    }
    catch (const std::exception& e) {
        std::cerr << "������ ��� ������ ��������: " << e.what() << std::endl;
    }
}

void GameController::SetSelectionStart(int x, int y) {
    selectionStart = Vector3d(x, y, 0.0f); // Z-���������� ������������� � 0, ��� ��� �������� � 2D
    selectionEnd = Vector3d(x, y, 0.0f); // ���������� ��� ����� ���������
    isSelectionActive = true;
}

void GameController::SetSelectionEnd(int x, int y) {
    selectionEnd = Vector3d(x, y, 0.0f); // ���������� ��� ����� ���������
    // ������� ������ ���������� ������ ����� ����������� �����
    ClearSelectedCells();

    int startX = static_cast<int>(selectionStart.X());
    int startY = static_cast<int>(selectionStart.Y());
    int endX = static_cast<int>(selectionEnd.X());
    int endY = static_cast<int>(selectionEnd.Y());

    // ���������� ����������� � ������������ ���������� ��� ����������� ���������
    int minX = (startX < endX) ? startX : endX;
    int maxX = (startX > endX) ? startX : endX;
    int minY = (startY < endY) ? startY : endY;
    int maxY = (startY > endY) ? startY : endY;

    // ��������� ��� ������ � �������� ����������� ��������������
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            if (x >= 0 && y >= 0 && x < grid.getWidth() && y < grid.getHeight()) { // �������� ������ �����
                AddSelectedCell(x, y);
            }
        }
    }
}

void GameController::AddSelectedCell(int x, int y) {
    selectedCells.push_back(Vector3d(x, y, 0.0f)); // ��������� ������ � ������ ����������, z ������ 0 � 2D
}

void GameController::ClearSelectedCells() {
    selectedCells.clear(); // ������� ������ ���������� ������
}

void GameController::KillSelectedCells() {
    for (const auto& cell : selectedCells) {
        int x = static_cast<int>(cell.X());
        int y = static_cast<int>(cell.Y());
        grid.setCellState(x, y, false);
        grid.setCellColor(x, y, 0.0f, 0.0f, 0.0f); // ������������� ���� ��� ������� ������
    }
    isSelectionActive = false;
}

void GameController::ReviveSelectedCells() {
    for (const auto& cell : selectedCells) {
        int x = static_cast<int>(cell.X());
        int y = static_cast<int>(cell.Y());
        grid.setCellState(x, y, true);
        grid.setCellColor(x, y, 0.1f, 0.4f, 0.1f); // ������������� ���� ��� ����� ������
    }
    isSelectionActive = false;
}