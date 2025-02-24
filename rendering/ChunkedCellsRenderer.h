#ifndef CHUNKED_CELLS_RENDERER_H
#define CHUNKED_CELLS_RENDERER_H

#include "BaseRenderer.h"

class ChunkedCellsRenderer : public BaseRenderer {
public:
    ChunkedCellsRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~ChunkedCellsRenderer() override;

    void Initialize() override;
    void Draw() override;

private:
    struct Chunk {
        int gridX, gridY; // Позиция чанка в сетке (в клетках)
        int width, height; // Размер чанка в клетках
        GLuint vao, vbo, instanceVBO; // Буферы OpenGL
    };

    int chunkSize; // Размер чанка в клетках (например, 100)
    std::vector<Chunk> chunks; // Список всех чанков
    GLuint cellShaderProgram; // Программа шейдера для клеток

    void SetupShaders(); // Настройка шейдеров
    void CreateChunk(int gridX, int gridY); // Создание одного чанка
    void RenderChunk(const Chunk& chunk); // Отрисовка чанка
    bool IsChunkInView(const Chunk& chunk) const; // Проверка видимости чанка
};

#endif // CHUNKED_CELLS_RENDERER_H