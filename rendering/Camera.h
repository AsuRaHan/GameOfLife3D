#pragma once
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

    void SetDirection(float dx, float dy, float dz); // ������������� ����������� ������
    void GetDirection(float& dx, float& dy, float& dz) const; // �������� ����������� ������

    void SetUpVector(float ux, float uy, float uz); // ������������� ������ "�����"
    void GetUpVector(float& ux, float& uy, float& uz) const; // �������� ������ "�����"

    void Move(float dx, float dy, float dz); // ����������� ������
    void Rotate(float yaw, float pitch); // �������� ������ (�������� � ������)

    void UpdateViewMatrix(); // ��������� ������� ����
    void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane); // ������������� ��������

    void LookAt(float targetX, float targetY, float targetZ, 
                float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f);

    float GetFOV() const { return fov; }

    const float* GetViewMatrix() const { return viewMatrix.data(); }
    const float* GetProjectionMatrix() const { return projectionMatrix.data(); }

    float GetDistance() const;
    //void SetFOV(float newFov);

    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float MAX_PITCH = 89.0f * PI / 180.0f;

private:
    std::array<float, 3> position;
    std::array<float, 3> direction;
    std::array<float, 3> up;
    float fov; // ���� ������ (FOV)

    std::array<float, 16> viewMatrix;
    std::array<float, 16> projectionMatrix;

    void NormalizeVector(std::array<float, 3>& vector); // �������� � float* �� std::array
};
#endif // CAMERA_H_