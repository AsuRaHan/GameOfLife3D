#ifndef RENDERER_H_
#define RENDERER_H_

#pragma once

#include "../system/GLFunctions.h"
#include "Camera.h"
#include "../game/GameController.h"
#include "UI.h"

#include <vector>



class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void SetCamera(const Camera& camera);
    Camera& GetCamera() { return camera; } // ������� const

    void SetGameController(GameController* gameController);


    void Draw();

    void OnWindowResize(int newWidth, int newHeight);

    void MoveCamera(float dx, float dy, float dz);

    int getWindowWidth() const { return width; }
    int getWindowHeight() const { return height; }
    bool getShowGrid() { return showGrid; };
    void setShowGrid(bool show) { showGrid = show; };

    void RebuildGameField(); // ����� ��� ����������� �������� ����

private:
    int width, height;
    Camera camera;
    GameController* pGameController;
    UI ui; // ��������� ��������� UI

    bool showGrid = true;
    ShaderManager shaderManager; // ��������� ShaderManager

    void InitializeVBOs();
    GLuint gridVBO;
    GLuint cellsVBO;
    std::vector<GLfloat> gridVertices;
    std::vector<GLfloat> cellVertices;

    struct CellInstance {
        float x, y; // ������� ������
        Vector3d color; // ���� ������
    };
    GLuint cellInstanceVBO;
    std::vector<CellInstance> cellInstances;
    GLuint shaderProgram;
    // ��� �������� �����
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
    std::string LoadShaderSource(const std::string& filename);

    void InitializeGridVBOs();

    void InitializeDebugOverlay();
    void UpdateDebugOverlayPosition();

   
};
#endif // RENDERER_H_