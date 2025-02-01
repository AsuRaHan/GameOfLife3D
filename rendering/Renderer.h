//#pragma once
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma once
#include "Camera.h"
#include "../game/GameController.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>


class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void SetCamera(const Camera& camera);
    //const Camera& GetCamera() const { return camera; }
    Camera& GetCamera() { return camera; } // Удалить const

    void SetGameController(GameController* gameController);


    void Draw();

    void OnWindowResize(int newWidth, int newHeight);

    void MoveCamera(float dx, float dy, float dz);

    int getWindowWidth() const { return width; }
    int getWindowHeight() const { return height; }

private:
    int width, height;
    Camera camera;
    GameController* pGameController;

    void InitializeVBOs();
    GLuint gridVBO;
    GLuint cellsVBO;
    std::vector<GLfloat> gridVertices;
    std::vector<GLfloat> cellVertices;

    struct CellInstance {
        float x, y; // Позиция клетки
        float state; // Состояние клетки (жив/мертв)
        Vector3d color; // Цвет клетки
        int type; // Тип клетки
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
    std::string LoadShaderSource(const std::string& filename);
    void CheckShaderCompilation(GLuint shader, const std::string& name);
    void CheckProgramLinking(GLuint program);

    void InitializeGridVBOs();

    void InitializeDebugOverlay();
    void UpdateDebugOverlayPosition();
};