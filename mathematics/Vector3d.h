#pragma once
#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <iostream>
#include <cmath>

class Vector3d {
public:
    // ������������
    Vector3d();
    Vector3d(float x, float y, float z);

    // �������� � ���������
    Vector3d operator+(const Vector3d& other) const;
    Vector3d operator-(const Vector3d& other) const;
    Vector3d operator*(float scalar) const;
    Vector3d operator/(float scalar) const;
    //Vector3d& operator=(const Vector3d& other) {
    //    if (this != &other) { // ������ �� ����������������
    //        x = other.x;
    //        y = other.y;
    //        z = other.z;
    //    }
    //    return *this;
    //}
    // ������
    float length() const;
    Vector3d normalize() const;

    // ����� �������
    void print() const;

    // ������ � �����������
    float X() const;
    float Y() const;
    float Z() const;

private:
    float x, y, z; // ���������� �������
};

#endif // VECTOR3D_H
