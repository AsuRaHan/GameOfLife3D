#pragma once
#ifndef RENDERER_H_
#define RENDERER_H_

#include "../system/GLFunctions.h"


#include "../game/GameController.h"
#include "IRendererProvider.h"
#include "Camera.h"

#include "UIRenderer.h"
#include "SelectionRenderer.h"
#include "GridRenderer.h"
#include "CellsRenderer.h"
#include "TextureFieldRenderer.h"
//#include "CubeRenderer.h"


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
    ShaderManager shaderManager;
    UIRenderer uiRenderer;
    SelectionRenderer selectionRenderer; // Добавляем экземпляр SelectionRenderer
    GridRenderer gridRenderer;
    CellsRenderer cellsRenderer;
    TextureFieldRenderer textureFieldRenderer;
    //CubeRenderer cubeRenderer;

    void SetupOpenGL();

};
#endif // RENDERER_H_