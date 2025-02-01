#include "Camera.h"
#include <iostream>

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : fov(fov) { // Инициализируем FOV
    // Инициализация позиции, направления и вектора "вверх"
    position[0] = 0.0f;
    position[1] = 0.0f;
    position[2] = 0.0f;

    direction[0] = 0.0f;
    direction[1] = 0.0f;
    direction[2] = -1.0f; // По умолчанию камера смотрит вдоль отрицательной оси Z

    up[0] = 0.0f;
    up[1] = 1.0f;
    up[2] = 0.0f; // Вектор "вверх" по оси Y

    // Инициализация матриц
    SetProjection(fov, aspectRatio, nearPlane, farPlane);
    UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z) {
    position[0] = x;
    position[1] = y;
    position[2] = z;
    UpdateViewMatrix();
}

void Camera::GetPosition(float& x, float& y, float& z) const {
    x = position[0];
    y = position[1];
    z = position[2];
}

void Camera::SetDirection(float dx, float dy, float dz) {
    direction[0] = dx;
    direction[1] = dy;
    direction[2] = dz;
    NormalizeVector(direction);
    UpdateViewMatrix();
}

void Camera::GetDirection(float& dx, float& dy, float& dz) const {
    dx = direction[0];
    dy = direction[1];
    dz = direction[2];
}

void Camera::SetUpVector(float ux, float uy, float uz) {
    up[0] = ux;
    up[1] = uy;
    up[2] = uz;
    NormalizeVector(up);
    UpdateViewMatrix();
}

void Camera::GetUpVector(float& ux, float& uy, float& uz) const {
    ux = up[0];
    uy = up[1];
    uz = up[2];
}

void Camera::Move(float dx, float dy, float dz) {
    position[0] += dx;
    position[1] += dy;
    position[2] += dz;
    UpdateViewMatrix();
}

void Camera::Rotate(float yaw, float pitch) {
    // Вращение по рысканию (yaw) и тангажу (pitch)
    float cosYaw = std::cos(yaw);
    float sinYaw = std::sin(yaw);
    float cosPitch = std::cos(pitch);
    float sinPitch = std::sin(pitch);

    // Обновляем направление камеры
    float newDirection[3];
    newDirection[0] = direction[0] * cosYaw - direction[2] * sinYaw;
    newDirection[2] = direction[0] * sinYaw + direction[2] * cosYaw;

    // Исправляем ограничение на pitch, чтобы камера не перевернулась
    float newPitch = direction[1] + pitch;
    if (newPitch > 89.0f * 3.14159265358979323846f / 180.0f) newPitch = 89.0f * 3.14159265358979323846f / 180.0f;
    if (newPitch < -89.0f * 3.14159265358979323846f / 180.0f) newPitch = -89.0f * 3.14159265358979323846f / 180.0f;

    newDirection[1] = std::sin(newPitch);

    NormalizeVector(newDirection);

    // Обновляем вектор направления
    direction[0] = newDirection[0];
    direction[1] = newDirection[1];
    direction[2] = newDirection[2];

    // Обновляем вектор "вверх"
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix() {
    // Вычисляем вектор "вправо" (right)
    float right[3];
    right[0] = up[1] * direction[2] - up[2] * direction[1];
    right[1] = up[2] * direction[0] - up[0] * direction[2];
    right[2] = up[0] * direction[1] - up[1] * direction[0];
    NormalizeVector(right);

    // Пересчитываем вектор "вверх" (up)
    float newUp[3];
    newUp[0] = direction[1] * right[2] - direction[2] * right[1];
    newUp[1] = direction[2] * right[0] - direction[0] * right[2];
    newUp[2] = direction[0] * right[1] - direction[1] * right[0];
    NormalizeVector(newUp);

    // Заполняем матрицу вида
    viewMatrix[0] = right[0];
    viewMatrix[1] = newUp[0];
    viewMatrix[2] = -direction[0];
    viewMatrix[3] = 0.0f;

    viewMatrix[4] = right[1];
    viewMatrix[5] = newUp[1];
    viewMatrix[6] = -direction[1];
    viewMatrix[7] = 0.0f;

    viewMatrix[8] = right[2];
    viewMatrix[9] = newUp[2];
    viewMatrix[10] = -direction[2];
    viewMatrix[11] = 0.0f;

    viewMatrix[12] = -(right[0] * position[0] + right[1] * position[1] + right[2] * position[2]);
    viewMatrix[13] = -(newUp[0] * position[0] + newUp[1] * position[1] + newUp[2] * position[2]);
    viewMatrix[14] = (direction[0] * position[0] + direction[1] * position[1] + direction[2] * position[2]);
    viewMatrix[15] = 1.0f;
}

void Camera::SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane) {
    float f = 1.0f / std::tan(fov * 0.5f * 3.14159265358979323846f / 180.0f);

    projectionMatrix[0] = f / aspectRatio;
    projectionMatrix[1] = 0.0f;
    projectionMatrix[2] = 0.0f;
    projectionMatrix[3] = 0.0f;

    projectionMatrix[4] = 0.0f;
    projectionMatrix[5] = f;
    projectionMatrix[6] = 0.0f;
    projectionMatrix[7] = 0.0f;

    projectionMatrix[8] = 0.0f;
    projectionMatrix[9] = 0.0f;
    projectionMatrix[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    projectionMatrix[11] = -1.0f;

    projectionMatrix[12] = 0.0f;
    projectionMatrix[13] = 0.0f;
    projectionMatrix[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
    projectionMatrix[15] = 0.0f;
}

void Camera::LookAt(float targetX, float targetY, float targetZ, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
    // Вектор направления (forward)
    float forwardX = targetX - position[0];
    float forwardY = targetY - position[1];
    float forwardZ = targetZ - position[2];
    float length = std::sqrt(forwardX * forwardX + forwardY * forwardY + forwardZ * forwardZ);
    if (length != 0) {
        forwardX /= length;
        forwardY /= length;
        forwardZ /= length;
    }

    // Вектор "вправо" (right)
    float up[3] = { upX, upY, upZ };
    float rightX = up[1] * forwardZ - up[2] * forwardY;
    float rightY = up[2] * forwardX - up[0] * forwardZ;
    float rightZ = up[0] * forwardY - up[1] * forwardX;
    length = std::sqrt(rightX * rightX + rightY * rightY + rightZ * rightZ);
    if (length != 0) {
        rightX /= length;
        rightY /= length;
        rightZ /= length;
    }

    // Новый вектор "вверх" (up)
    float newUpX = forwardY * rightZ - forwardZ * rightY;
    float newUpY = forwardZ * rightX - forwardX * rightZ;
    float newUpZ = forwardX * rightY - forwardY * rightX;

    // Заполняем матрицу вида
    viewMatrix[0] = rightX;
    viewMatrix[1] = newUpX;
    viewMatrix[2] = -forwardX;
    viewMatrix[3] = 0.0f;

    viewMatrix[4] = rightY;
    viewMatrix[5] = newUpY;
    viewMatrix[6] = -forwardY;
    viewMatrix[7] = 0.0f;

    viewMatrix[8] = rightZ;
    viewMatrix[9] = newUpZ;
    viewMatrix[10] = -forwardZ;
    viewMatrix[11] = 0.0f;

    viewMatrix[12] = -(rightX * position[0] + rightY * position[1] + rightZ * position[2]);
    viewMatrix[13] = -(newUpX * position[0] + newUpY * position[1] + newUpZ * position[2]);
    viewMatrix[14] = (forwardX * position[0] + forwardY * position[1] + forwardZ * position[2]);
    viewMatrix[15] = 1.0f;
}

void Camera::NormalizeVector(float* vector) {
    float length = std::sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
    if (length != 0) {
        vector[0] /= length;
        vector[1] /= length;
        vector[2] /= length;
    }
}