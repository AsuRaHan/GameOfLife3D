#ifndef CELLS_RENDERER_H
#define CELLS_RENDERER_H

#include "BaseRenderer.h"

struct CellInstance {
    float x, y;
};

class CellsRenderer : public BaseRenderer {
public:
    CellsRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~CellsRenderer();

    void Initialize();
    void Draw();

private:
    GLuint cellsVAO, cellsVBO, cellInstanceVBO;
    GLuint cellShaderProgram, computeCellShaderProgram;
    std::vector<CellInstance> cellInstances;

    //std::vector<CellInstance> visibleCellInstances; // ������ ������� ����������
    //int visibleCellsCount; // ���������� ������� ������

    void LoadCellShaders();
    void CreateOrUpdateCellInstancesUsingComputeShader(std::vector<CellInstance>& cellInstances);
    //void UpdateVisibleCells(); // ����� ��� ���������� ������� ������
};

#endif