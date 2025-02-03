#include "GameOfLife.h"

GameOfLife::GameOfLife(Grid& g) : grid(g), nextGrid(g.getWidth(), g.getHeight()) {
    //std::srand(static_cast<unsigned int>(std::time(nullptr))); // ������������� ���������� ��������� �����
}

int GameOfLife::countLiveNeighbors(int x, int y) const {
    static const int offsets[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };

    int count = 0;
    for (const auto& offset : offsets) {
        int nx = (x + offset[0] + grid.getWidth()) % grid.getWidth();
        int ny = (y + offset[1] + grid.getHeight()) % grid.getHeight();

        if (grid.getCellState(nx, ny)) {
            count++;
        }
    }
    return count;
}

int GameOfLife::countLiveNeighborsWorld(int x, int y) const {
    int count = 0;

    // ��������� ��� 8 �������
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            // ���������� ���� ������
            if (i == 0 && j == 0) continue;

            int nx = x + i;
            int ny = y + j;

            // ���������, ��������� �� ���������� � �������� ������ �����
            if (nx >= 0 && nx < grid.getWidth() && ny >= 0 && ny < grid.getHeight()) {
                if (grid.getCellState(nx, ny)) {
                    count++;
                }
            }
        }
    }
    return count;
}

void GameOfLife::nextGeneration() {
    //saveCurrentState(); // ��������� ������� ���������
    int neighbors;
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            if (isToroidal) {
                neighbors = countLiveNeighbors(x, y);
            }
            else {
                neighbors = countLiveNeighborsWorld(x, y);
            }

            bool currentState = grid.getCellState(x, y);
            Cell& oldCell = grid.getCell(x, y); // ������ �� ������ ������
            Cell newCell = oldCell; // �������� ������ ������

            if (currentState) {
                // ����� ������
                newCell.setAlive(neighbors == 2 || neighbors == 3);
                if (!newCell.getAlive()) {
                    // ���� ������ �������, ���������� �����-����� ����
                    newCell.setColor(Vector3d(0.25f, 0.25f, 0.25f));
                }
                else {
                    // ���� ������ �������� �����, ������ � ������� �����
                    newCell.setColor(Vector3d(0.3f, 0.8f, 0.3f));
                }
            }
            else {
                // ������� ������
                newCell.setAlive(neighbors == 3);
                if (newCell.getAlive()) {
                    // ���� ������ ���������, ������ � ����� �������� �����
                    newCell.setColor(Vector3d(0.0f, 0.5f, 0.0f)); // ������� ����
                }
            }
            nextGrid.setCell(x, y, newCell); // ������������� ����� ������
        }
    }
    std::swap(grid, nextGrid); // ����������� ������
}

void GameOfLife::previousGeneration() {
    if (!history.empty()) {
        grid = history.back(); // ��������������� ���������� ���������
        history.pop_back(); // ������� ��������� ����������� ��������� �� �������
    }
    else {
        // ��������� ������, ����� ��� ���������� ���������
        //std::cout << "��� ����������� ��������� ��� ��������������." << std::endl;
    }
}

void GameOfLife::saveCurrentState() {
    if (history.size() >= 100) {
        history.erase(history.begin()); // ������� ����� ������ ���������
    }
    history.push_back(grid); // ��������� ������� ���������
}