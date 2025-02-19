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
    // для шейдеров сетки
    GLuint gridShaderProgram;
    int gridVertexCount;
    int gridBufferSize;

    void InitializeCellsVBOs();
    GLuint cellsVBO;
    GLuint cellsVAO;
    std::vector<GLfloat> gridVertices;

    GLuint cellInstanceVBO;
    struct CellInstance {
        float x, y; // Позиция клетки
    };
    std::vector<CellInstance> cellInstances;
    GLuint cellShaderProgram;

    void SetupOpenGL();

    void DrawGrid();
    void DrawCells();

    void LoadShaders();
	void LoadCellShaders();
    void LoadGridShaders();

    GLuint computeCellShaderProgram;
    GLuint computeGridShaderProgram;

    void LoadComputeShader();

    void CreateOrUpdateCellInstancesUsingComputeShader(std::vector<CellInstance>& cellInstances);

    void CreateOrUpdateGridVerticesUsingComputeShader(std::vector<float>& gridVertices);
};
#endif // RENDERER_H_