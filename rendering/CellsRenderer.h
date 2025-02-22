#ifndef CELLS_RENDERER_H
#define CELLS_RENDERER_H

#include "BaseRenderer.h"

//#include "../system/GLFunctions.h"
//#include "../system/ShaderManager.h"
//
//#include <vector>
//#include <iostream>

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

    void LoadCellShaders();
    void CreateOrUpdateCellInstancesUsingComputeShader(std::vector<CellInstance>& cellInstances);
};

#endif