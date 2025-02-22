#ifndef TEXTURE_FIELD_RENDERER_H
#define TEXTURE_FIELD_RENDERER_H

#include "BaseRenderer.h"

class TextureFieldRenderer : public BaseRenderer {
public:
    TextureFieldRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~TextureFieldRenderer() override;

    void Initialize() override;
    void Draw() override;

private:
    GLuint VAO, VBO, textureID;
    GLuint textureShaderProgram;

    void LoadTextureShaders();
};


#endif