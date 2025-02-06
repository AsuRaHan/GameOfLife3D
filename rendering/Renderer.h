#pragma once
#ifndef RENDERER_H_
#define RENDERER_H_

#include "../system/GLFunctions.h"
#include "Camera.h"
#include "UIController.h"
#include "../game/GameController.h"
#include <vector>
#include <memory>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h" 

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void SetCamera(const Camera& camera);
    Camera& GetCamera() { return camera; }

    void SetGameController(GameController* gameController);


    void Draw();

    void OnWindowResize(int newWidth, int newHeight);

    void MoveCamera(float dx, float dy, float dz);

    int getWindowWidth() const { return width; }
    int getWindowHeight() const { return height; }
    bool getShowGrid() { return showGrid; };
    void setShowGrid(bool show) { showGrid = show; };

    void RebuildGameField(); // метод для перестройки игрового поля

    void initializeUI();

private:
    int width, height;
    Camera camera;
    GameController* pGameController;
    UIController uiController;

    bool showGrid = true;
    ShaderManager shaderManager; // Добавляем ShaderManager

    
    void InitializeGridVBOs();
    GLuint gridVBO;
    GLuint gridVAO;

    void InitializeVBOs();
    GLuint cellsVBO;
    GLuint cellsVAO;

    std::vector<GLfloat> gridVertices;
    std::vector<GLfloat> cellVertices;

    struct CellInstance {
        float x, y; // Позиция клетки
        Vector3d color; // Цвет клетки
    };
    GLuint cellInstanceVBO;
    std::vector<CellInstance> cellInstances;
    GLuint shaderProgram;
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