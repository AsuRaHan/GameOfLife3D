#include "Renderer.h"

Renderer::Renderer(int width, int height)
    : width(width), height(height), farPlane(999009000000.0f), camera(45.0f, static_cast<float>(width) / height, 0.1f, farPlane),
    pGameController(nullptr), uiRenderer(nullptr), 
    selectionRenderer(camera, shaderManager), 
    gridRenderer(camera, shaderManager),
    cellsRenderer(camera, shaderManager),
    textureFieldRenderer(camera, shaderManager)
    //,cubeRenderer(camera, shaderManager)
{
    SetupOpenGL();
    OnWindowResize(width, height);
}

Renderer::~Renderer() {

}


void Renderer::SetupOpenGL() {
    GL_CHECK(glClearColor(0.06f, 0.0f, 0.06f, 1.0f)); // Черный фон
    GL_CHECK(glEnable(GL_DEPTH_TEST)); // Включаем тест глубины
    GL_CHECK(glViewport(0, 0, width, height));
}

void Renderer::OnWindowResize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    GL_CHECK(glViewport(0, 0, width, height));
    // Обновляем проекцию камеры
    camera.SetProjection(45.0f, static_cast<float>(width) / height, 0.1f, farPlane);
}


void Renderer::SetCamera(const Camera& camera) {
    this->camera = camera;    
}

void Renderer::SetGameController(GameController* gameController) {
    pGameController = gameController;

    uiRenderer = UIRenderer(pGameController);
    uiRenderer.InitializeUI();

    selectionRenderer.SetGameController(gameController);
    gridRenderer.SetGameController(gameController);
    cellsRenderer.SetGameController(gameController);
    textureFieldRenderer.SetGameController(gameController);
    //cubeRenderer.SetGameController(gameController);

    // Инициализируем буферы после установки GameController
    selectionRenderer.Initialize();
    gridRenderer.Initialize();
    cellsRenderer.Initialize();
    textureFieldRenderer.Initialize();
    //cubeRenderer.Initialize();

}

void Renderer::Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Проверяем расстояние камеры
    float cameraDistance = camera.GetDistance();
    // Всегда рисуем выделение, если активно
    selectionRenderer.Draw();
    if (cameraDistance < 300.0f) {
        // Камера ближе 300: рисуем сетку и ячейки
        cellsRenderer.Draw();
    }
    else {
        // Камера дальше 300: рисуем поле как текстуру
        textureFieldRenderer.Draw();
    }
    gridRenderer.Draw();
    //if (cameraDistance < 40.0f) {
    //    cubeRenderer.Draw(); // Кубы рисуются всегда поверх всего
    //}


    // Отрисовка UI, если включено
    if (pGameController && pGameController->getShowUI()) {
        uiRenderer.DrawUI();
    }

    SwapBuffers(wglGetCurrentDC());
}

void Renderer::RebuildGameField() {
    if (!pGameController) return;
    cellsRenderer.Initialize();
    gridRenderer.Initialize();
    textureFieldRenderer.Initialize();
    //cubeRenderer.Initialize();
}