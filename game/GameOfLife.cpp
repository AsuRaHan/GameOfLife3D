#include "GameOfLife.h"

GameOfLife::GameOfLife(Grid& g) : grid(g), nextGrid(g.getWidth(), g.getHeight()),
gpuAutomaton(g.getWidth(), g.getHeight()), isToroidal(true), isGpuSimulated(true),
cellInstances(nullptr),cellProvider(nullptr), gridReferenceIsUbdated(true)
{
    GW = grid.getWidth();
    GH = grid.getHeight();
    gpuAutomaton.SetToroidal(isToroidal); // ��������� isToroidal
    gpuAutomaton.SetNewGridSize(GW, GH);
    currentState.resize(GW * GH);
    nextState.resize(GW * GH);
    
}

int GameOfLife::countLiveNeighbors(int x, int y) const {
    constexpr int offsets[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };

    int count = 0;
    for (const auto& offset : offsets) {
        int nx = x + offset[0];
        int ny = y + offset[1];

        // ���������, ��������� �� ���������� � �������� ������ �����
        if (nx >= 0 && nx < grid.getWidth() && ny >= 0 && ny < grid.getHeight()) {
            if (grid.getCellState(nx, ny)) {
                count++;
            }
        }
    }
    return count;
}

int GameOfLife::countLiveNeighborsWorldToroidal(int x, int y) const {
    constexpr int offsets[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };

    int count = 0;
    const int w = grid.getWidth();
    const int h = grid.getHeight();

    for (const auto& offset : offsets) {
        int nx = x + offset[0];
        int ny = y + offset[1];

        // ������������� ��������� ��� ������������ ����� ��� ������������� wrap
        nx = (nx < 0) ? w - 1 : (nx >= w ? 0 : nx);
        ny = (ny < 0) ? h - 1 : (ny >= h ? 0 : ny);

        if (grid.getCellState(nx, ny)) {
            count++;
        }
    }
    return count;
}

void GameOfLife::setWoldToroidal(bool wt) {
    isToroidal = wt;
    gpuAutomaton.SetToroidal(isToroidal); // ��������� isToroidal � GPUAutomaton
}

void GameOfLife::nextGeneration() {
    if (isGpuSimulated) {
        nextGenerationGPU();
    }
    else {
        nextGenerationCPU();
    }
}

void GameOfLife::updateGridReference(Grid& newGrid) {
    grid = newGrid;
    GW = grid.getWidth();
    GH = grid.getHeight();

    currentState.resize(GW * GH);
    nextState.resize(GW * GH);
    gpuAutomaton.SetNewGridSize(GW, GH);

}

void GameOfLife::nextGenerationGPU() {
    //for (int y = 0; y < GH; ++y) {
    //    for (int x = 0; x < GW; ++x) {
    //        currentState[y * GW + x] = grid.getCellState(x, y) ? 1 : 0;
    //    }
    //}

    //����������� �������� ������ �� GPU
    auto future = std::async(std::launch::async, [&]() {
        for (int y = 0; y < GH; ++y) {
            for (int x = 0; x < GW; ++x) {
                currentState[y * GW + x] = grid.getCellState(x, y) ? 1 : 0;
            }
        }
        });

    // ������ �������� ���� ������ �����������
    future.wait(); // ���� ���������� ��������

    gpuAutomaton.SetGridState(currentState);
    // ��������� ���������� �� GPU
    gpuAutomaton.Update();
    // �������� ����� ��������� � GPU
    gpuAutomaton.GetGridState(nextState);


    for (auto y = 0; y < GH; ++y) {
        for (auto x = 0; x < GW; ++x) {
            bool newState = nextState[y * GW + x] != 0;
            Cell& cell = grid.getCell(x, y);
            bool oldState = cell.getAlive();

            if (newState != oldState) { // ������ ���� ��������� ����������
                cell.setAlive(newState);
                if (newState) {
                    // ����� ��������� ������ �������� ������ ����
                    cell.setColor(Vector3d(0.0f, 0.4f, 0.0f)); // ��������
                }
                else {
                    // ������� ������ �������� �����-����� ����
                    cell.setColor(Vector3d(0.05f, 0.05f, 0.08f)); // ������
                }
                SetCellColor(x, y, cell.getColor());
            }
            else if (newState) {
                // ���� ������ �������� ���, ������ � �������
                Vector3d currentColor = cell.getColor();
                // ����������� ��� ���������� ����� �� ��������� ��������, �� �� ����� 1.0
                float newR = currentColor.X() + 0.05f;
                float newG = currentColor.Y() + 0.05f;
                float newB = currentColor.Z() + 0.05f;
                newR = newR > 0.3f ? 0.3f : newR;
                newG = newG > 0.9f ? 0.9f : newG;
                newB = newB > 0.3f ? 0.3f : newB;
                cell.setColor(Vector3d(newR, newG, newB));
                SetCellColor(x, y, cell.getColor());
            }
        }
    }

}

void GameOfLife::SetCellColor(int x, int y, const Vector3d& color) {
    if (cellProvider && cellInstances) {
        //int GW = grid.getWidth();
        int index = y * GW + x;
        if (index < cellInstances->size()) {
            (*cellInstances)[index].color = color;
        }
    }
}

void GameOfLife::SetCellProvider(const IRendererProvider* provider) {
    cellProvider = provider;
    if (cellProvider) {
        cellInstances = &const_cast<std::vector<CellInstance>&>(cellProvider->GetCellInstances());
    }
    else {
        cellInstances = nullptr;
    }
}

void GameOfLife::nextGenerationCPU() {
    //saveCurrentState(); // ��������� ������� ���������
    int neighbors;
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            if (isToroidal) {                
                neighbors = countLiveNeighborsWorldToroidal(x, y);
            }
            else {
                neighbors = countLiveNeighbors(x, y);
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