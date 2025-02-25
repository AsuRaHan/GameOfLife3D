#pragma once
#ifndef PATTERN_INSERTER_RENDERER_H_
#define PATTERN_INSERTER_RENDERER_H_

#include "BaseRenderer.h"


class PatternInserterRenderer : public BaseRenderer {
public:
    PatternInserterRenderer(const Camera& camera, ShaderManager& shaderManager);
    ~PatternInserterRenderer();

    void Initialize() override;
    void Draw() override;

private:
    void LoadShaders();
    void UpdateBuffers(); // Обновляем вершины на основе паттерна
    void UpdateOutlineBuffers(); // метод для контура

    GLuint VAO, VBO;
    GLuint outlineVAO, outlineVBO; // Для контура

    std::vector<std::vector<bool>> currentPattern; // Текущий паттерн (локальная копия)
    int cellType; // Тип клеток для цвета
    Vector3d previewPosition; // Позиция предпросмотра
    GLuint patternShaderProgram;
    std::vector<float> vertices; // Динамический массив вершин
    std::vector<float> outlineVertices; // Вершины контура

};

#endif // PATTERN_INSERTER_RENDERER_H_