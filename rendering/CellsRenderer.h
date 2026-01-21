#ifndef CELLS_RENDERER_H
#define CELLS_RENDERER_H

#include "BaseRenderer.h"

class CellsRenderer : public BaseRenderer {
public:
    CellsRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~CellsRenderer();
    void Initialize() override;
    void Draw() override;
    virtual void Rebuild() override;
private:
    struct CellInstance {
        float x, y, z;  // position
        float pad;      // padding for alignment (to make sizeof=32 bytes)
        float r, g, b, a;  // color
    };
    GLuint cellsVAO, cellsVBO, cellsIBO, cellInstanceVBO;
    GLuint cellShaderProgram;
    GLuint computeShaderProgram;
    GLuint visibleCellsBuffer; // SSBO для хранения видимых позиций (vec4: x,y,z,pad)
    GLuint visibleColorsBuffer; // SSBO для хранения цветов видимых клеток
    std::vector<CellInstance> visibleCellInstances;
    void LoadShaders();
    void UpdateVisibleCells();
    bool IsCellVisible(float x, float y) const;
    void GetVisibleGridBounds(int& minX, int& minY, int& maxX, int& maxY) const;
    void LoadComputeShader();
};

#endif