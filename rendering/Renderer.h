#pragma once
#ifndef RENDERER_H_
#define RENDERER_H_

#include "../system/GLFunctions.h"
#include "Camera.h"
#include "UIRenderer.h"
#include "../game/GameController.h"
#include "IRendererProvider.h"
//#include "CameraController.h"

#include "CubeRenderer.h"

#include <vector>
#include <memory>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h" 

class Renderer : public IRendererProvider {
public:
    Renderer(int width, int height);
    ~Renderer();

    void SetCamera(const Camera& camera);
    Camera& GetCamera() override { return camera; } // смотри интерфейс IRendererProvider

    void SetGameController(GameController* gameController);
    void Draw();
    void OnWindowResize(int newWidth, int newHeight);
    //void MoveCamera(float dx, float dy, float dz);
    int getWindowWidth() const { return width; }
    int getWindowHeight() const { return height; }
    void RebuildGameField() override; // метод для перестройки игрового поля. смотри интерфейс IRendererProvider

    void DrawCubes();

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
    //CameraController cameraController;

    GameController* pGameController;
    UIRenderer uiRenderer;
    ShaderManager shaderManager;

    CubeRenderer cubeRenderer;
    
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
};
#endif // RENDERER_H_