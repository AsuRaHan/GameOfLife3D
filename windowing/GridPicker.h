#pragma once
#include "../game/GameController.h"
#include "../rendering/Camera.h"

class GridPicker {
public:
    GridPicker(const Camera& camera);

    void ScreenToWorld(float screenX, float screenY, float screenWidth, float screenHeight, float& worldX, float& worldY);

private:
    const Camera& camera;
    //GameController& gameController;

    void MultiplyMatrixVector(const float* matrix, const float* vector, float* result) const;
    void InvertMatrix(const float* m, float* out) const;
    float DotProduct(const float* a, const float* b) const;
    void Normalize(float* v) const;
};