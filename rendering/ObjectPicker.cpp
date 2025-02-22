#include "ObjectPicker.h"
#include <cmath>
#include <algorithm>

ObjectPicker::ObjectPicker(const Camera& camera, GameController* gameController)
    : camera(camera), pGameController(gameController) {}

void ObjectPicker::MultiplyMatrixVector(const float* matrix, const float* vector, float* result) const {
    for (int i = 0; i < 4; ++i) {
        result[i] = 0;
        for (int j = 0; j < 4; ++j) {
            result[i] += matrix[i * 4 + j] * vector[j];
        }
    }
}

bool ObjectPicker::InvertMatrix(const float* m, float* out) const {
    // Твой код инверсии матрицы (оставляем как есть)
    float inv[16], det;
    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (det == 0) return false;
    det = 1.0f / det;
    for (int i = 0; i < 16; i++) out[i] = inv[i] * det;
    return true;
}

float ObjectPicker::DotProduct(const float* a, const float* b) const {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void ObjectPicker::Normalize(float* v) const {
    float length = std::sqrt(DotProduct(v, v));
    if (length != 0) {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
}

void ObjectPicker::GetRayFromScreen(float screenX, float screenY, float screenWidth, float screenHeight, Vector3d& rayOrigin, Vector3d& rayDirection) {
    float x = (2.0f * screenX / screenWidth) - 1.0f;
    float y = 1.0f - (2.0f * screenY / screenHeight);

    float ndcRay[4] = { x, y, -1.0f, 1.0f };
    float invProjection[16], invView[16], out[4];
    if (!InvertMatrix(camera.GetProjectionMatrix(), invProjection) || !InvertMatrix(camera.GetViewMatrix(), invView)) {
        rayOrigin = Vector3d(0, 0, 0);
        rayDirection = Vector3d(0, 0, 1);
        return;
    }

    MultiplyMatrixVector(invProjection, ndcRay, out);
    float eyeRay[4] = { out[0] * out[3], out[1] * out[3], -1.0f, 0.0f };
    MultiplyMatrixVector(invView, eyeRay, out);
    rayDirection = Vector3d(out[0], out[1], out[2]);
    Normalize(&rayDirection.x);

    float camPos[3];
    camera.GetPosition(camPos[0], camPos[1], camPos[2]);
    rayOrigin = Vector3d(camPos[0], camPos[1], camPos[2]);
}

bool ObjectPicker::IntersectGrid(const Vector3d& rayOrigin, const Vector3d& rayDirection, Vector3d& hitPoint, int& cellX, int& cellY) {
    Vector3d planeNormal(0, 0, 1);
    Vector3d planePoint(0, 0, 0);
    float denom = planeNormal.Dot(rayDirection);
    if (std::abs(denom) < 1e-6f) return false;

    float t = (planePoint - rayOrigin).Dot(planeNormal) / denom;
    if (t < 0) return false;

    hitPoint = rayOrigin + rayDirection * t;
    float cellSize = pGameController->getCellSize();
    float gridWidth = pGameController->getGridWidth() * cellSize;
    float gridHeight = pGameController->getGridHeight() * cellSize;

    if (hitPoint.x < 0 || hitPoint.x >= gridWidth || hitPoint.y < 0 || hitPoint.y >= gridHeight) return false;

    cellX = static_cast<int>(hitPoint.x / cellSize);
    cellY = static_cast<int>(hitPoint.y / cellSize);
    return true;
}

bool ObjectPicker::IntersectCube(const Vector3d& rayOrigin, const Vector3d& rayDirection, const Vector3d& cubePos, float cubeSize, Vector3d& hitPoint) {
    Vector3d minBounds = cubePos - Vector3d(cubeSize / 2);
    Vector3d maxBounds = cubePos + Vector3d(cubeSize / 2);

    float tMin = -std::numeric_limits<float>::infinity();
    float tMax = std::numeric_limits<float>::infinity();

    for (int i = 0; i < 3; ++i) {
        if (std::abs(rayDirection[i]) < 1e-6f) {
            if (rayOrigin[i] < minBounds[i] || rayOrigin[i] > maxBounds[i]) return false;
        } else {
            float t1 = (minBounds[i] - rayOrigin[i]) / rayDirection[i];
            float t2 = (maxBounds[i] - rayOrigin[i]) / rayDirection[i];
            tMin = std::max(tMin, std::min(t1, t2));
            tMax = std::min(tMax, std::max(t1, t2));
        }
    }

    if (tMin > tMax || tMax < 0) return false;
    float t = tMin > 0 ? tMin : tMax;
    hitPoint = rayOrigin + rayDirection * t;
    return true;
}

bool ObjectPicker::IntersectSphere(const Vector3d& rayOrigin, const Vector3d& rayDirection, const Vector3d& center, float radius, Vector3d& hitPoint) {
    Vector3d oc = rayOrigin - center;
    float a = rayDirection.Dot(rayDirection);
    float b = 2.0f * oc.Dot(rayDirection);
    float c = oc.Dot(oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) return false;

    float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    if (t < 0) {
        t = (-b + std::sqrt(discriminant)) / (2.0f * a);
        if (t < 0) return false;
    }

    hitPoint = rayOrigin + rayDirection * t;
    return true;
}

bool ObjectPicker::IntersectTriangle(const Vector3d& rayOrigin, const Vector3d& rayDirection, const MeshTriangle& triangle, Vector3d& hitPoint) {
    Vector3d edge1 = triangle.v1 - triangle.v0;
    Vector3d edge2 = triangle.v2 - triangle.v0;
    Vector3d h = rayDirection.Cross(edge2);
    float a = edge1.Dot(h);

    if (std::abs(a) < 1e-6f) return false; // Луч параллелен треугольнику

    float f = 1.0f / a;
    Vector3d s = rayOrigin - triangle.v0;
    float u = f * s.Dot(h);
    if (u < 0.0f || u > 1.0f) return false;

    Vector3d q = s.Cross(edge1);
    float v = f * rayDirection.Dot(q);
    if (v < 0.0f || u + v > 1.0f) return false;

    float t = f * edge2.Dot(q);
    if (t < 0) return false;

    hitPoint = rayOrigin + rayDirection * t;
    return true;
}

int ObjectPicker::PickObject(float screenX, float screenY, float screenWidth, float screenHeight) {
    Vector3d rayOrigin, rayDirection;
    GetRayFromScreen(screenX, screenY, screenWidth, screenHeight, rayOrigin, rayDirection);

    Vector3d hitPoint, closestHit;
    int closestId = -1;
    float closestT = std::numeric_limits<float>::infinity();

    // Пересечение с сеткой
    int cellX, cellY;
    if (IntersectGrid(rayOrigin, rayDirection, hitPoint, cellX, cellY)) {
        int gridWidth = pGameController->getGridWidth();
        if (cellX >= 0 && cellX < gridWidth && cellY >= 0 && cellY < pGameController->getGridHeight()) {
            float t = (hitPoint - rayOrigin).Length() / rayDirection.Length();
            if (t < closestT) {
                closestT = t;
                closestId = cellY * gridWidth + cellX;
                closestHit = hitPoint;
            }
        }
    }

    // Пример: пересечение со сферами (нужно получить список сфер из GameController)
    // std::vector<Sphere> spheres = pGameController->GetSpheres();
    // for (const auto& sphere : spheres) {
    //     if (IntersectSphere(rayOrigin, rayDirection, sphere.center, sphere.radius, hitPoint)) {
    //         float t = (hitPoint - rayOrigin).Length() / rayDirection.Length();
    //         if (t < closestT) {
    //             closestT = t;
    //             closestId = sphere.id;
    //             closestHit = hitPoint;
    //         }
    //     }
    // }

    // Пример: пересечение с мешем (нужно получить список треугольников)
    // std::vector<MeshTriangle> triangles = pGameController->GetMeshTriangles();
    // for (const auto& triangle : triangles) {
    //     if (IntersectTriangle(rayOrigin, rayDirection, triangle, hitPoint)) {
    //         float t = (hitPoint - rayOrigin).Length() / rayDirection.Length();
    //         if (t < closestT) {
    //             closestT = t;
    //             closestId = triangle.id;
    //             closestHit = hitPoint;
    //         }
    //     }
    // }

    return closestId;
}