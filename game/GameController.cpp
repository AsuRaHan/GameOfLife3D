#include "GameController.h"
#include <random> // ��� ��������� ��������� �����


GameController::GameController(int width, int height, float cellSize)
    : grid(width, height), gameOfLife(grid), cellSize(cellSize), isRunning(false) {
}

void GameController::initializeGrid() {
    // ������������� ��������� ����� ������ ��� ������������
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            // ������� ������� ���� ��� ������������
            if ((x == 40 && y == 30) ||  // ��������� ������
                (x == 41 && y == 30) ||  // �������� ������
                (x == 42 && y == 30) ||  // ��� ���� �������� ������
                (x == 41 && y == 31)) {  // ������ ����� ��� ������������ "�����"
                grid.setCellState(x, y, true);
            }
        }
    }
}

void GameController::randomizeGrid(float density) {
    // ��������� ��������� �����
    std::random_device rd;  // ������ ��� ������������� ����������
    std::mt19937 gen(rd()); // ����������� ������������� �������
    std::uniform_real_distribution<> dis(0.0, 1.0); // ����������� �������������

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            // ���� ��������� ����� ������ density, ������ ���������� �����
            if (dis(gen) < density) {
                grid.setCellState(x, y, true);
            }
            else {
                grid.setCellState(x, y, false);
            }
        }
    }
}

void GameController::update() {
    if (isRunning) {
        gameOfLife.nextGeneration();
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
}

bool GameController::getCellState(int x, int y) const {
    return grid.getCellState(x, y);
}