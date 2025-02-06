#include "Camera.h"
#include <iostream>

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : fov(fov),
      position{0.0f, 0.0f, 0.0f},
      direction{0.0f, 0.0f, -1.0f},
      up{0.0f, 1.0f, 0.0f} {
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
    float cosYaw = std::cos(yaw);
    float sinYaw = std::sin(yaw);
    float cosPitch = std::cos(pitch);
    float sinPitch = std::sin(pitch);

    std::array<float, 3> newDirection = {
        direction[0] * cosYaw - direction[2] * sinYaw,
        direction[1],
        direction[0] * sinYaw + direction[2] * cosYaw
    };

    float newPitch = direction[1] + pitch;
    if (newPitch > MAX_PITCH) 
        newPitch = MAX_PITCH;
    if (newPitch < -MAX_PITCH) 
        newPitch = -MAX_PITCH;

    newDirection[1] = std::sin(newPitch);
    NormalizeVector(newDirection);
    direction = newDirection;
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix() {
    std::array<float, 3> right = {
        up[1] * direction[2] - up[2] * direction[1],
        up[2] * direction[0] - up[0] * direction[2],
        up[0] * direction[1] - up[1] * direction[0]
    };
    NormalizeVector(right);

    std::array<float, 3> newUp = {
        direction[1] * right[2] - direction[2] * right[1],
        direction[2] * right[0] - direction[0] * right[2],
        direction[0] * right[1] - direction[1] * right[0]
    };
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
    float f = 1.0f / std::tan(fov * 0.5f * PI / 180.0f);

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

void Camera::LookAt(float targetX, float targetY, float targetZ, float upX, float upY, float upZ) {
    // Вектор направления (forward)
    std::array<float, 3> forward = {
        targetX - position[0],
        targetY - position[1],
        targetZ - position[2]
    };
    float length = std::sqrt(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
    if (length != 0) {
        forward[0] /= length;
        forward[1] /= length;
        forward[2] /= length;
    }

    // Вектор "вправо" (right)
    std::array<float, 3> tempUp = { upX, upY, upZ };
    std::array<float, 3> right = {
        tempUp[1] * forward[2] - tempUp[2] * forward[1],
        tempUp[2] * forward[0] - tempUp[0] * forward[2],
        tempUp[0] * forward[1] - tempUp[1] * forward[0]
    };
    NormalizeVector(right);

    // Новый вектор "вверх" (up)
    std::array<float, 3> newUp = {
        forward[1] * right[2] - forward[2] * right[1],
        forward[2] * right[0] - forward[0] * right[2],
        forward[0] * right[1] - forward[1] * right[0]
    };
    NormalizeVector(newUp);

    // Заполняем матрицу вида
    viewMatrix[0] = right[0];
    viewMatrix[1] = newUp[0];
    viewMatrix[2] = -forward[0];
    viewMatrix[3] = 0.0f;

    viewMatrix[4] = right[1];
    viewMatrix[5] = newUp[1];
    viewMatrix[6] = -forward[1];
    viewMatrix[7] = 0.0f;

    viewMatrix[8] = right[2];
    viewMatrix[9] = newUp[2];
    viewMatrix[10] = -forward[2];
    viewMatrix[11] = 0.0f;

    viewMatrix[12] = -(right[0] * position[0] + right[1] * position[1] + right[2] * position[2]);
    viewMatrix[13] = -(newUp[0] * position[0] + newUp[1] * position[1] + newUp[2] * position[2]);
    viewMatrix[14] = (forward[0] * position[0] + forward[1] * position[1] + forward[2] * position[2]);
    viewMatrix[15] = 1.0f;
}

void Camera::NormalizeVector(std::array<float, 3>& vector) {
    float length = std::sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
    if (length != 0) {
        vector[0] /= length;
        vector[1] /= length;
        vector[2] /= length;
    }
}

float Camera::GetDistance() const {
    return std::abs(position[2]);
}