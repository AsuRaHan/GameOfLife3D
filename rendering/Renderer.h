#pragma once
#ifndef RENDERER_H_
#define RENDERER_H_

#include "../system/GLFunctions.h"
#include "Camera.h"
#include "../game/GameController.h"
#include "GUI/UIManager.h"
#include "GUI/Button.h"
#include "GUI/Label.h"
#include <vector>
#include <memory>


class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void SetCamera(const Camera& camera);
    Camera& GetCamera() { return camera; } // Удалить const

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
    void OnMouseClick(int x, int y);
    void OnMouseMove(int x, int y);

private:
    int width, height;
    Camera camera;
    GameController* pGameController;

    bool showGrid = true;
    ShaderManager shaderManager; // Добавляем ShaderManager

    void InitializeVBOs();
    GLuint gridVBO;
    GLuint cellsVBO;
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
    GLuint gridVAO;

    GLuint debugOverlayShaderProgram, debugOverlayVAO, debugOverlayVBO, debugOverlayTexture;

    void SetupOpenGL();
    void DrawGrid();
    void DrawCells();
    void DrawDebugOverlay();

    void LoadShaders();
	void LoadCellShaders();
    void LoadGridShaders();
    

    void InitializeGridVBOs();

    void InitializeDebugOverlay();
    void UpdateDebugOverlayPosition();

    std::shared_ptr<UIManager> uiManager;
};
#endif // RENDERER_H_