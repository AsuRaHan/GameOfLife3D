#ifndef GRID_RENDERER_H
#define GRID_RENDERER_H

#include "BaseRenderer.h"

class GridRenderer : public BaseRenderer {
public:
    GridRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~GridRenderer();

    void Initialize();
    void Draw();
    virtual void Rebuild() override;
private:
    GLuint gridVAO, gridVBO;
    GLuint gridShaderProgram;
    std::vector<float> gridVertices;
    int gridVertexCount;
    size_t gridBufferSize;
    GLuint computeGridShaderProgram;

    void LoadGridShaders();
    void CreateOrUpdateGridVerticesUsingComputeShader(std::vector<float>& gridVertices);
};

#endif