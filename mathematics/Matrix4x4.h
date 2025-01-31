#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#include <array>
#include <iostream>
#include <iomanip>

class Matrix4x4 {
public:
    // ����������� (��������� �������)
    Matrix4x4();

    // ����������� � ����������
    Matrix4x4(const std::array<float, 16>& elements);

    // ����������� �����������
    Matrix4x4(const Matrix4x4& other);

    // �������� ������������
    Matrix4x4& operator=(const Matrix4x4& other);

    // �������� �������� ������
    Matrix4x4 operator+(const Matrix4x4& other) const;

    // �������� ��������� ������
    Matrix4x4 operator-(const Matrix4x4& other) const;

    // �������� ��������� ������
    Matrix4x4 operator*(const Matrix4x4& other) const;

    // ���������������� �������
    Matrix4x4 transpose() const;

    // �������� ������� (��������)
    Matrix4x4 inverse() const;
    float determinant() const;
    float minor(int row, int col) const;
    // ����� �������
    void print() const;

    // ��������� ��������� �� ������
    const float* data() const;

private:
    std::array<float, 16> elements; // �������� �������
};

#endif // MATRIX4X4_H
