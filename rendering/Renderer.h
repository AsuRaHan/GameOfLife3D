//#pragma once
//#include <Windows.h>
//#include <gl/GL.h>

#pragma once
#include "Camera.h"
#include "../game/GameController.h"
#include "DebugOverlay.h"
#include <vector>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void SetCamera(const Camera& camera);
    //const Camera& GetCamera() const { return camera; }
    Camera& GetCamera() { return camera; } // Удалить const

    void SetGameController(GameController* gameController);
    void SetDebugOverlay(DebugOverlay* debugOverlay);

    void Draw();

    void OnWindowResize(int newWidth, int newHeight);

    void MoveCamera(float dx, float dy, float dz);

    int getWindowWidth() const { return width; }
    int getWindowHeight() const { return height; }

private:
    int width, height;
    Camera camera;
    GameController* pGameController;
    DebugOverlay* pDebugOverlay;
    void InitializeVBOs();
    GLuint gridVBO;
    GLuint cellsVBO;
    std::vector<GLfloat> gridVertices;
    std::vector<GLfloat> cellVertices;

    void SetupOpenGL();
    void DrawGrid();
    void DrawCells();
    void DrawDebugOverlay();
};