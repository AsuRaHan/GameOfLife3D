#include "Vector3d.h"
#include <stdexcept> // Для std::runtime_error

// Конструкторы
Vector3d::Vector3d() : x(0.0f), y(0.0f), z(0.0f) {}

Vector3d::Vector3d(float x, float y, float z) : x(x), y(y), z(z) {}

// Операции с векторами
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
    if (scalar == 0) {
        throw std::runtime_error("Division by zero in Vector3d::operator/");
    }
    return Vector3d(x / scalar, y / scalar, z / scalar);
}

// Методы
float Vector3d::length() const {
    auto lenSquared = x * x + y * y + z * z;
    return std::sqrt(lenSquared);
}

Vector3d Vector3d::normalize() const {
    float len = length();
    if (len > 0) {
        return *this / len; // Делим на длину для нормализации
    }
    return *this; // Возвращаем нулевой вектор, если длина равна 0
}

// Вывод вектора
void Vector3d::print() const {
    std::cout << "Vector3d(" << x << ", " << y << ", " << z << ")\n";
}

// Доступ к компонентам
float Vector3d::X() const { return x; }
float Vector3d::Y() const { return y; }
float Vector3d::Z() const { return z; }
