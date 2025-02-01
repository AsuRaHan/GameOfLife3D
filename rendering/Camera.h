#pragma once
#include <cmath>

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

    void LookAt(float targetX, float targetY, float targetZ, float upX, float upY, float upZ);

    float GetFOV() const { return fov; }

    const float* GetViewMatrix() const { return viewMatrix; }
    const float* GetProjectionMatrix() const { return projectionMatrix; }

private:
    float position[3]; // ������� ������
    float direction[3]; // ����������� ������
    float up[3]; // ������ "�����"
    float fov; // ���� ������ (FOV)

    float viewMatrix[16]; // ������� ����
    float projectionMatrix[16]; // ������� ��������

    void NormalizeVector(float* vector); // ������������ �������
};