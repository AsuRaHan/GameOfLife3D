#pragma once
#ifndef RENDERER_H_
#define RENDERER_H_

#include "../system/GLFunctions.h"
#include "Camera.h"
#include "UIRenderer.h"
#include "SelectionRenderer.h"
#include "../game/GameController.h"
#include "IRendererProvider.h"

#include <vector>
#include <memory>

class Renderer : public IRendererProvider {
public:
    Renderer(int width, int height);
    ~Renderer();

    void SetCamera(const Camera& camera);
    Camera& GetCamera() override { return camera; } // смотри интерфейс IRendererProvider

    void SetGameController(GameController* gameController);
    void Draw() override;
    void OnWindowResize(int newWidth, int newHeight) override;
    int getWindowWidth() const { return width; }
    int getWindowHeight() const { return height; }
    void RebuildGameField() override; // метод для перестройки игрового поля. смотри интерфейс IRendererProvider

    // Геттер для cellInstances
    const std::vector<CellInstance>& GetCellInstances() const override {
        return cellInstances;
    }

    // Сеттер для cellInstances
    void SetCellInstances(const std::vector<CellInstance>& instances) { cellInstances = instances; }

private:
    int width, height;
    Camera camera;
    float farPlane;

    GameController* pGameController;
    UIRenderer uiRenderer;
    ShaderManager shaderManager;
    SelectionRenderer selectionRenderer; // Добавляем экземпляр SelectionRenderer
    void InitializeGridVBOs();
    GLuint gridVBO;
    GLuint gridVAO;

    void InitializeCellsVBOs();
    GLuint cellsVBO;
    GLuint cellsVAO;

    std::vector<GLfloat> gridVertices;
    GLuint cellInstanceVBO;
    std::vector<CellInstance> cellInstances;

    GLuint cellShaderProgram;
    // для шейдеров сетки
    GLuint gridShaderProgram;

    GLuint debugOverlayShaderProgram, debugOverlayVAO, debugOverlayVBO, debugOverlayTexture;

    void SetupOpenGL();

    void DrawGrid();
    void DrawCells();

    void LoadShaders();
	void LoadCellShaders();
    void LoadGridShaders();

    // Для отладочной текстуры
    GLuint debugTextureShaderProgram;
    GLuint debugTextureVAO;
    GLuint debugTextureVBO;
    GLuint debugTextureID; // ID текстуры
    void DrawDebugTexture(); // Новый метод для отрисовки текстуры
    void LoadDebugTextureShaders(); // Новый метод для загрузки шейдеров текстуры
    void InitializeDebugTexture();
};
#endif // RENDERER_H_