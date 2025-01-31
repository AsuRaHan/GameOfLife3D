#include "Vector3d.h"

// ������������
Vector3d::Vector3d() : x(0.0f), y(0.0f), z(0.0f) {}

Vector3d::Vector3d(float x, float y, float z) : x(x), y(y), z(z) {}

// �������� � ���������
Vector3d Vector3d::operator+(const Vector3d& other) const {
    return Vector3d(x + other.x, y + other.y, z + other.z);
}

Vector3d Vector3d::operator-(const Vector3d& other) const {
    return Vector3d(x - other.x, y - other.y, z - other.z);
}

Vector3d Vector3d::operator*(float scalar) const {
    return Vector3d(x * scalar, y * scalar, z * scalar);
}

Vector3d Vector3d::operator/(float scalar) const {
    if (scalar != 0) {
        return Vector3d(x / scalar, y / scalar, z / scalar);
    }
    return Vector3d(); // ���������� ������� ������, ���� ����� �� 0
}

// ������
float Vector3d::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vector3d Vector3d::normalize() const {
    float len = length();
    if (len > 0) {
        return *this / len; // ����� �� ����� ��� ������������
    }
    return *this; // ���������� ������� ������, ���� ����� ����� 0
}

// ����� �������
void Vector3d::print() const {
    std::cout << "Vector3d(" << x << ", " << y << ", " << z << ")\n";
}

// ������ � �����������
float Vector3d::X() const { return x; }
float Vector3d::Y() const { return y; }
float Vector3d::Z() const { return z; }
