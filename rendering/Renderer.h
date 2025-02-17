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
    Camera& GetCamera() override { return camera; } // ������ ��������� IRendererProvider

    void SetGameController(GameController* gameController);
    void Draw() override;
    void OnWindowResize(int newWidth, int newHeight) override;
    int getWindowWidth() const { return width; }
    int getWindowHeight() const { return height; }
    void RebuildGameField() override; // ����� ��� ����������� �������� ����. ������ ��������� IRendererProvider

    // ������ ��� cellInstances
    const std::vector<CellInstance>& GetCellInstances() const override {
        return cellInstances;
    }

    // ������ ��� cellInstances
    void SetCellInstances(const std::vector<CellInstance>& instances) { cellInstances = instances; }

private:
    int width, height;
    Camera camera;
    float farPlane;

    GameController* pGameController;
    UIRenderer uiRenderer;
    ShaderManager shaderManager;
    SelectionRenderer selectionRenderer; // ��������� ��������� SelectionRenderer
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
    // ��� �������� �����
    GLuint gridShaderProgram;

    GLuint debugOverlayShaderProgram, debugOverlayVAO, debugOverlayVBO, debugOverlayTexture;

    void SetupOpenGL();

    void DrawGrid();
    void DrawCells();

    void LoadShaders();
	void LoadCellShaders();
    void LoadGridShaders();

    // ��� ���������� ��������
    GLuint debugTextureShaderProgram;
    GLuint debugTextureVAO;
    GLuint debugTextureVBO;
    GLuint debugTextureID; // ID ��������
    void DrawDebugTexture(); // ����� ����� ��� ��������� ��������
    void LoadDebugTextureShaders(); // ����� ����� ��� �������� �������� ��������
    void InitializeDebugTexture();
};
#endif // RENDERER_H_