#include "Renderer.h"

Renderer::Renderer(int width, int height)
    : width(width), height(height), farPlane(999009000000.0f), camera(45.0f, static_cast<float>(width) / height, 0.1f, farPlane),
    pGameController(nullptr), uiRenderer(nullptr), 
    selectionRenderer(camera, shaderManager), 
    gridRenderer(camera, shaderManager),
    textureFieldRenderer(camera, shaderManager)
    , cellsViewportRenderer(camera, shaderManager)
    , cellsRenderer(camera, shaderManager)
    , patternInserterRenderer(camera, shaderManager)
{
    SetupOpenGL();
    OnWindowResize(width, height);
}

Renderer::~Renderer() {

}

void Renderer::SetupOpenGL() {
    GL_CHECK(glClearColor(0.03f, 0.0f, 0.03f, 1.0f)); // Черный фон
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

void Renderer::SetGameController(GameController* gameController) {
    pGameController = gameController;

    uiRenderer = UIRenderer(pGameController);

    selectionRenderer.SetGameController(gameController);
    gridRenderer.SetGameController(gameController);
    textureFieldRenderer.SetGameController(gameController);
    cellsViewportRenderer.SetGameController(gameController);
    cellsRenderer.SetGameController(gameController);
    patternInserterRenderer.SetGameController(gameController);

    // Инициализируем буферы после установки GameController
    selectionRenderer.Initialize();
    gridRenderer.Initialize();
    textureFieldRenderer.Initialize();
    cellsViewportRenderer.Initialize();
    cellsRenderer.Initialize();
    patternInserterRenderer.Initialize();
}

void Renderer::Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Проверяем расстояние камеры
    float cameraDistance = camera.GetDistance();
    // Всегда рисуем выделение, если активно
    selectionRenderer.Draw();
    if (pGameController && pGameController->IsPatternPlacementMode()) {
        patternInserterRenderer.Draw();
    }
    
    if (cameraDistance < 20.0f) {
        // Камера ближе 300: рисуем ячейки
        cellsRenderer.Draw();
    }
    else if (cameraDistance < 300.0f) {
        cellsViewportRenderer.Draw();
    }
    else{
        // Камера дальше 300: рисуем поле как текстуру
        textureFieldRenderer.Draw();
    }
    gridRenderer.Draw();

    // Отрисовка UI, если включено
    if (pGameController && pGameController->getShowUI()) {
        uiRenderer.DrawUI();
    }

    SwapBuffers(wglGetCurrentDC());
}

void Renderer::RebuildGameField() {
    if (!pGameController) return;
    gridRenderer.Rebuild();
    textureFieldRenderer.Rebuild();
    cellsViewportRenderer.Initialize();
    cellsRenderer.Initialize();
}