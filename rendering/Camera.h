#pragma once
#ifndef CAMERA_H_
#define CAMERA_H_

#include <cmath>
#include <array>

class Camera {
public:
    Camera(float fov, float aspectRatio, float nearPlane, float farPlane);

    void SetPosition(float x, float y, float z);
    void GetPosition(float& x, float& y, float& z) const;

    void SetDirection(float dx, float dy, float dz); // Устанавливаем направление камеры
    void GetDirection(float& dx, float& dy, float& dz) const; // Получаем направление камеры

    void SetUpVector(float ux, float uy, float uz); // Устанавливаем вектор "вверх"
    void GetUpVector(float& ux, float& uy, float& uz) const; // Получаем вектор "вверх"

    void Move(float dx, float dy, float dz); // Перемещение камеры
    void Rotate(float yaw, float pitch); // Вращение камеры (рыскание и тангаж)

    void UpdateViewMatrix(); // Обновляем матрицу вида
    void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane); // Устанавливаем проекцию

    void LookAt(float targetX, float targetY, float targetZ, 
                float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f);

    float GetFOV() const { return fov; }
    float GetFarPlane() const { return fPlane; }
    const float* GetViewMatrix() const { return viewMatrix.data(); }
    const float* GetProjectionMatrix() const { return projectionMatrix.data(); }

    float GetDistance() const;

    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float MAX_PITCH = 89.0f * PI / 180.0f;

private:
    std::array<float, 3> position;
    std::array<float, 3> direction;
    std::array<float, 3> up;
    float fov; // Угол обзора (FOV)
    float fPlane;
    std::array<float, 16> viewMatrix;
    std::array<float, 16> projectionMatrix;

    void NormalizeVector(std::array<float, 3>& vector); // Изменено с float* на std::array
};
#endif // CAMERA_H_