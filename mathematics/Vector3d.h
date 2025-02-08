#pragma once
#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <iostream>
#include <cmath>

class Vector3d {
public:
    // Конструкторы
    Vector3d();
    Vector3d(float x, float y, float z);

    // Операции с векторами
    Vector3d operator+(const Vector3d& other) const;
    Vector3d operator-(const Vector3d& other) const;
    Vector3d operator*(float scalar) const;
    Vector3d operator/(float scalar) const;
    //Vector3d& operator=(const Vector3d& other) {
    //    if (this != &other) { // защита от самоприсваивания
    //        x = other.x;
    //        y = other.y;
    //        z = other.z;
    //    }
    //    return *this;
    //}
    // Методы
    float length() const;
    Vector3d normalize() const;

    // Вывод вектора
    void print() const;

    // Доступ к компонентам
    float X() const;
    float Y() const;
    float Z() const;

private:
    float x, y, z; // Компоненты вектора
};

#endif // VECTOR3D_H
