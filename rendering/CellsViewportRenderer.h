#ifndef CELLS_VIEWPORT_RENDERER_H
#define CELLS_VIEWPORT_RENDERER_H

#include "BaseRenderer.h"



class CellsViewportRenderer : public BaseRenderer {
public:
    CellsViewportRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~CellsViewportRenderer();

    void Initialize() override;
    void Draw() override;
    virtual void Rebuild() override;
private:
    struct CellInstance {
        float x, y;
    };
    GLuint cellsVAO, cellsVBO, cellInstanceVBO;
    GLuint cellShaderProgram;
    std::vector<CellInstance> visibleCellInstances;
    GLuint computeShaderProgram;
    GLuint visibleCellsBuffer;    // SSBO для хранения видимых клеток
    GLuint visibleColorsBuffer;   // SSBO для хранения цветов видимых клеток

    void LoadShaders();
    void UpdateVisibleCells();
    bool IsCellVisible(float x, float y) const;
    void GetVisibleGridBounds(int& minX, int& minY, int& maxX, int& maxY) const;
    void LoadComputeShader();
};

#endif 