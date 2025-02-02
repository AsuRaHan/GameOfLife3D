#include "GridPicker.h"


//GridPicker::GridPicker(const Camera& camera, GameController& gameController)
//    : camera(camera), gameController(gameController) {
//}
GridPicker::GridPicker(const Camera& camera) : camera(camera) {}

void GridPicker::MultiplyMatrixVector(const float* matrix, const float* vector, float* result) const {
    for (int i = 0; i < 4; ++i) {
        result[i] = 0;
        for (int j = 0; j < 4; ++j) {
            result[i] += matrix[i * 4 + j] * vector[j];
        }
    }
}

void GridPicker::InvertMatrix(const float* m, float* out) const {
    // Простое инвертирование матрицы 4x4 - не самый эффективный метод, но рабочий
    float inv[16], det;
    int i;

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

    if (det == 0)
        throw "Determinant is zero!";

    det = 1.0f / det;

    for (i = 0; i < 16; i++)
        out[i] = inv[i] * det;
}

float GridPicker::DotProduct(const float* a, const float* b) const {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void GridPicker::Normalize(float* v) const {
    float length = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (length != 0) {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
}

void GridPicker::ScreenToWorld(float screenX, float screenY, float screenWidth, float screenHeight, float& worldX, float& worldY) {
    // Преобразование экранных координат в нормализованные устройственные координаты (NDC)
    float x = (2.0f * screenX / screenWidth) - 1.0f;
    float y = 1.0f - (2.0f * screenY / screenHeight);
    float z = 1.0f; // Для ближней плоскости отсечения

    float ndcRay[4] = { x, y, -1.0f, 1.0f }; // Отрицательное z для направления луча вперед

    float invProjection[16], invView[16], out[4];
    InvertMatrix(camera.GetProjectionMatrix(), invProjection);
    InvertMatrix(camera.GetViewMatrix(), invView);

    // Преобразуем NDC в мировые координаты
    MultiplyMatrixVector(invProjection, ndcRay, out);
    float worldRay[4] = { out[0], out[1], -1.0f, 0.0f }; // Четвертый элемент 0 для вектора направления

    MultiplyMatrixVector(invView, worldRay, out);

    float direction[3] = { out[0], out[1], out[2] };
    Normalize(direction);

    // Находим точку пересечения с плоскостью z=0 (плоскость сетки)
    float camPos[3];
    camera.GetPosition(camPos[0], camPos[1], camPos[2]);
    float t = -camPos[2] / direction[2];

    worldX = camPos[0] + t * direction[0];
    worldY = camPos[1] + t * direction[1];
}