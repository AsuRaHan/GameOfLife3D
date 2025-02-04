#pragma once
#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#include <array>
#include <iostream>
#include <iomanip>

class Matrix4x4 {
public:
    // Конструктор (единичная матрица)
    Matrix4x4();

    // Конструктор с элементами
    Matrix4x4(const std::array<float, 16>& elements);

    // Конструктор копирования
    Matrix4x4(const Matrix4x4& other);

    // Оператор присваивания
    Matrix4x4& operator=(const Matrix4x4& other);

    // Операция сложения матриц
    Matrix4x4 operator+(const Matrix4x4& other) const;

    // Операция вычитания матриц
    Matrix4x4 operator-(const Matrix4x4& other) const;

    // Операция умножения матриц
    Matrix4x4 operator*(const Matrix4x4& other) const;

    // Транспонирование матрицы
    Matrix4x4 transpose() const;

    // Обратная матрица (заглушка)
    Matrix4x4 inverse() const;
    float determinant() const;
    float minor(int row, int col) const;
    // Вывод матрицы
    void print() const;

    // Получение указателя на данные
    const float* data() const;

private:
    std::array<float, 16> elements; // Элементы матрицы
};

#endif // MATRIX4X4_H
