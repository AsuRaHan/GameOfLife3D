#pragma once
#include <cmath>

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

    void LookAt(float targetX, float targetY, float targetZ, float upX, float upY, float upZ);

    float GetFOV() const { return fov; }

    const float* GetViewMatrix() const { return viewMatrix; }
    const float* GetProjectionMatrix() const { return projectionMatrix; }

private:
    float position[3]; // Позиция камеры
    float direction[3]; // Направление камеры
    float up[3]; // Вектор "вверх"
    float fov; // Угол обзора (FOV)

    float viewMatrix[16]; // Матрица вида
    float projectionMatrix[16]; // Матрица проекции

    void NormalizeVector(float* vector); // Нормализация вектора
};