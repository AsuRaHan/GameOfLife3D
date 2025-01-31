#include "Renderer.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <iostream>

#include "../system/GLFunctions.h"

Renderer::Renderer(int width, int height)
    : width(width), height(height), camera(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f),
    pGameController(nullptr), pDebugOverlay(nullptr) {
    SetupOpenGL();
}

Renderer::~Renderer() {
    // Очистка ресурсов, если необходимо
}

void Renderer::SetCamera(const Camera& camera) {
    this->camera = camera;
}

void Renderer::SetGameController(GameController* gameController) {
    this->pGameController = gameController;
    // Инициализация VBO
    InitializeVBOs();
}

void Renderer::SetDebugOverlay(DebugOverlay* debugOverlay) {
    this->pDebugOverlay = debugOverlay;
}

void Renderer::SetupOpenGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Черный фон
    glEnable(GL_DEPTH_TEST); // Включаем тест глубины
    glViewport(0, 0, width, height);
}
void Renderer::InitializeVBOs() {
    glGenBuffers(1, &cellsVBO);
    glGenBuffers(1, &gridVBO);

    if (!pGameController) return;
    gridVertices.clear(); // Очищаем вектор перед заполнением

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize(); // Получаем размер ячейки

    // Сбор данных для сетки
    for (int y = 0; y <= gridHeight; ++y) {
        // Горизонтальные линии
        gridVertices.push_back(0.0f); // Начало линии по X
        gridVertices.push_back(y * cellSize); // Y координата
        gridVertices.push_back(gridWidth * cellSize); // Конец линии по X
        gridVertices.push_back(y * cellSize); // Y координата
    }
    for (int x = 0; x <= gridWidth; ++x) {
        // Вертикальные линии
        gridVertices.push_back(x * cellSize); // X координата
        gridVertices.push_back(0.0f); // Начало линии по Y
        gridVertices.push_back(x * cellSize); // X координата
        gridVertices.push_back(gridHeight * cellSize); // Конец линии по Y
    }


}

//void Renderer::InitializeVBOs() {
//    // Создание VBO для сетки
//    glGenBuffers(1, &gridVBO);
//    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
//
//    // Заполнение данных для сетки
//    int gridWidth = pGameController->getGridWidth();
//    int gridHeight = pGameController->getGridHeight();
//    float cellSize = pGameController->getCellSize();
//
//    for (int y = 0; y <= gridHeight; ++y) {
//        gridVertices.push_back(0);
//        gridVertices.push_back(y * cellSize);
//        gridVertices.push_back(0);
//        gridVertices.push_back(gridWidth * cellSize);
//        gridVertices.push_back(y * cellSize);
//        gridVertices.push_back(0);
//    }
//    for (int x = 0; x <= gridWidth; ++x) {
//        gridVertices.push_back(x * cellSize);
//        gridVertices.push_back(0);
//        gridVertices.push_back(0);
//        gridVertices.push_back(x * cellSize);
//        gridVertices.push_back(gridHeight * cellSize);
//        gridVertices.push_back(0);
//    }
//
//    // Копируем данные в VBO
//    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(GLfloat), gridVertices.data(), GL_STATIC_DRAW);
//
//    // Создание VBO для клеток
//    glGenBuffers(1, &cellsVBO);
//    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);
//
//    // Заполнение данных для клеток
//    for (int y = 0; y < gridHeight; ++y) {
//        for (int x = 0; x < gridWidth; ++x) {
//            if (pGameController->getCellState(x, y)) {
//                // Добавляем вершины для квадрата (клетки)
//                cellVertices.push_back(x * cellSize);
//                cellVertices.push_back(y * cellSize);
//                cellVertices.push_back(0);
//                cellVertices.push_back((x + 1) * cellSize);
//                cellVertices.push_back(y * cellSize);
//                cellVertices.push_back(0);
//                cellVertices.push_back((x + 1) * cellSize);
//                cellVertices.push_back((y + 1) * cellSize);
//                cellVertices.push_back(0);
//                cellVertices.push_back(x * cellSize);
//                cellVertices.push_back((y + 1) * cellSize);
//                cellVertices.push_back(0);
//            }
//        }
//    }
//
//    // Копируем данные в VBO
//    glBufferData(GL_ARRAY_BUFFER, cellVertices.size() * sizeof(GLfloat), cellVertices.data(), GL_STATIC_DRAW);
//}

void Renderer::Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Устанавливаем матрицу проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMultMatrixf(camera.GetProjectionMatrix());

    // Устанавливаем видовую матрицу
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixf(camera.GetViewMatrix());

    // Отрисовка сетки и клеток
    DrawGrid();
    DrawCells();

    // Отрисовка осей для отладки
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f); // Красный для X
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(100.0f, 0.0f, 0.0f);

    glColor3f(0.0f, 1.0f, 0.0f); // Зеленый для Y
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 100.0f, 0.0f);

    glColor3f(0.0f, 0.0f, 1.0f); // Синий для Z
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();

    // Отрисовка отладочного оверлея
    if (pDebugOverlay) {
        DrawDebugOverlay();
    }

    SwapBuffers(wglGetCurrentDC());
}

//void Renderer::DrawGrid() {
//    if (!pGameController) return;
//
//    glColor3f(0.5f, 0.5f, 0.5f);
//    glBegin(GL_LINES);
//
//    int gridWidth = pGameController->getGridWidth();
//    int gridHeight = pGameController->getGridHeight();
//    float cellSize = pGameController->getCellSize();
//
//    for (int y = 0; y <= gridHeight; ++y) {
//        glVertex3f(0, y * cellSize, 0);
//        glVertex3f(gridWidth * cellSize, y * cellSize, 0);
//    }
//    for (int x = 0; x <= gridWidth; ++x) {
//        glVertex3f(x * cellSize, 0, 0);
//        glVertex3f(x * cellSize, gridHeight * cellSize, 0);
//    }
//
//    glEnd();
//}
void Renderer::DrawGrid() {
    // Обновляем данные в VBO для сетки
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW);

    // Отрисовка сетки
    glColor3f(0.5f, 0.5f, 0.5f); // Цвет для сетки
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(gridVertices.size() / 2)); // Отрисовка линий
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//void Renderer::DrawCells() {
//    if (!pGameController) return;
//
//    glBegin(GL_QUADS);
//
//    int gridWidth = pGameController->getGridWidth();
//    int gridHeight = pGameController->getGridHeight();
//    float cellSize = pGameController->getCellSize();
//
//    for (int y = 0; y < gridHeight; ++y) {
//        for (int x = 0; x < gridWidth; ++x) {
//            if (pGameController->getCellState(x, y)) {
//                glColor3f(0.0f, 0.6f, 0.0f); // Зеленый цвет для живых клеток
//                glVertex3f(x * cellSize, y * cellSize, 0);
//                glVertex3f((x + 1) * cellSize, y * cellSize, 0);
//                glVertex3f((x + 1) * cellSize, (y + 1) * cellSize, 0);
//                glVertex3f(x * cellSize, (y + 1) * cellSize, 0);
//            }
//        }
//    }
//
//    glEnd();
//}

void Renderer::DrawCells() {
    if (!pGameController) return;

    cellVertices.clear(); // Очищаем вектор перед заполнением

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize(); // Получаем размер ячейки

    // Сбор данных для клеток
    for (int i = 0; i < gridWidth; ++i) {
        for (int j = 0; j < gridHeight; ++j) {
            if (pGameController->getCellState(i, j)) {
                // Добавляем вершины для квадрата (клетки)
                cellVertices.push_back(i * cellSize); // Левый нижний угол
                cellVertices.push_back(j * cellSize);
                cellVertices.push_back((i + 1) * cellSize); // Правый нижний угол
                cellVertices.push_back(j * cellSize);
                cellVertices.push_back((i + 1) * cellSize); // Правый верхний угол
                cellVertices.push_back((j + 1) * cellSize);
                cellVertices.push_back(i * cellSize); // Левый верхний угол
                cellVertices.push_back((j + 1) * cellSize);
            }
        }
    }

    // Обновляем данные в VBO для клеток
    glBindBuffer(GL_ARRAY_BUFFER, cellsVBO);
    glBufferData(GL_ARRAY_BUFFER, cellVertices.size() * sizeof(float), cellVertices.data(), GL_DYNAMIC_DRAW);

    // Отрисовка живых клеток
    glColor3f(0.0f, 0.6f, 0.0f); // Зеленый цвет для живых клеток
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(cellVertices.size() / 2)); // Отрисовка клеток
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Renderer::DrawDebugOverlay() {
    if (!pDebugOverlay) return;
    pDebugOverlay->setSimulationRunning(pGameController->isSimulationRunning());
    pDebugOverlay->draw();
}

void Renderer::OnWindowResize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    glViewport(0, 0, width, height);

    // Обновляем проекцию камеры
    camera.SetProjection(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f);

    if (pDebugOverlay) {
        pDebugOverlay->onWindowResize(static_cast<float>(width), static_cast<float>(height));
    }
}

void Renderer::MoveCamera(float dx, float dy, float dz) {
    // Обновляем позицию камеры
    float currentX, currentY, currentZ;
    camera.GetPosition(currentX, currentY, currentZ);
    camera.SetPosition(currentX + dx, currentY + dy, currentZ + dz);
}