#pragma once
#ifndef OBJECT_PICKER_H_
#define OBJECT_PICKER_H_

#include "Camera.h"
#include "GameController.h"
#include "../mathematics/Vector3d.h"
#include <vector>

struct MeshTriangle {
    Vector3d v0, v1, v2; // Вершины треугольника
    int id;              // Уникальный ID объекта
};

struct Sphere {
    Vector3d center;
    float radius;
    int id;
};

class ObjectPicker {
public:
    ObjectPicker(const Camera& camera, GameController* gameController);

    void GetRayFromScreen(float screenX, float screenY, float screenWidth, float screenHeight, Vector3d& rayOrigin, Vector3d& rayDirection);
    int PickObject(float screenX, float screenY, float screenWidth, float screenHeight);

private:
    const Camera& camera;
    GameController* pGameController;

    void MultiplyMatrixVector(const float* matrix, const float* vector, float* result) const;
    bool InvertMatrix(const float* m, float* out) const;
    float DotProduct(const float* a, const float* b) const;
    void Normalize(float* v) const;

    bool IntersectGrid(const Vector3d& rayOrigin, const Vector3d& rayDirection, Vector3d& hitPoint, int& cellX, int& cellY);
    bool IntersectCube(const Vector3d& rayOrigin, const Vector3d& rayDirection, const Vector3d& cubePos, float cubeSize, Vector3d& hitPoint);
    bool IntersectSphere(const Vector3d& rayOrigin, const Vector3d& rayDirection, const Vector3d& center, float radius, Vector3d& hitPoint);
    bool IntersectTriangle(const Vector3d& rayOrigin, const Vector3d& rayDirection, const MeshTriangle& triangle, Vector3d& hitPoint);
};

#endif // OBJECT_PICKER_H_