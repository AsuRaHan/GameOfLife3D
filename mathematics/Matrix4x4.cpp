#include "Matrix4x4.h"

// Конструктор (единичная матрица)
Matrix4x4::Matrix4x4() {
    elements = { 1.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f };
}

// Конструктор с элементами
Matrix4x4::Matrix4x4(const std::array<float, 16>& elements) : elements(elements) {}

// Конструктор копирования
Matrix4x4::Matrix4x4(const Matrix4x4& other) : elements(other.elements) {}

// Оператор присваивания
Matrix4x4& Matrix4x4::operator=(const Matrix4x4& other) {
    if (this != &other) {
        elements = other.elements;
    }
    return *this;
}

// Операция сложения матриц
Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const {
    Matrix4x4 result;
    for (int i = 0; i < 16; ++i) {
        result.elements[i] = elements[i] + other.elements[i];
    }
    return result;
}

// Операция вычитания матриц
Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const {
    Matrix4x4 result;
    for (int i = 0; i < 16; ++i) {
        result.elements[i] = elements[i] - other.elements[i];
    }
    return result;
}

// Операция умножения матриц
Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
    Matrix4x4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.elements[i * 4 + j] =
                elements[i * 4 + 0] * other.elements[0 * 4 + j] +
                elements[i * 4 + 1] * other.elements[1 * 4 + j] +
                elements[i * 4 + 2] * other.elements[2 * 4 + j] +
                elements[i * 4 + 3] * other.elements[3 * 4 + j];
        }
    }
    return result;
}

// Транспонирование матрицы
Matrix4x4 Matrix4x4::transpose() const {
    Matrix4x4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
                result.elements[i * 4 + j] = elements[j * 4 + i];
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::inverse() const {
    float det = determinant();
    if (det == 0) {
        throw std::runtime_error("Matrix is not invertible");
    }

    Matrix4x4 adjugate;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            adjugate.elements[j * 4 + i] = minor(i, j) * ((i + j) % 2 == 0 ? 1 : -1);
        }
    }

    // Делим адъюнкт матрицы на определитель
    for (int i = 0; i < 16; ++i) {
        adjugate.elements[i] /= det;
    }

    return adjugate;
}

float Matrix4x4::determinant() const {
    float det = 0.0f;
    for (int i = 0; i < 4; ++i) {
        det += elements[i] * minor(0, i) * (i % 2 == 0 ? 1 : -1);
    }
    return det;
}

float Matrix4x4::minor(int row, int col) const {
    std::array<float, 9> submatrix;
    int submatrixIndex = 0;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i != row && j != col) {
                submatrix[submatrixIndex++] = elements[i * 4 + j];
            }
        }
    }

    // Определитель 3x3
    return submatrix[0] * (submatrix[4] * submatrix[8] - submatrix[5] * submatrix[7]) -
        submatrix[1] * (submatrix[3] * submatrix[8] - submatrix[5] * submatrix[6]) +
        submatrix[2] * (submatrix[3] * submatrix[7] - submatrix[4] * submatrix[6]);
}


// Вывод матрицы
void Matrix4x4::print() const {
    for (int i = 0; i < 4; ++i) {
        std::cout << "| ";
        for (int j = 0; j < 4; ++j) {
            std::cout << std::setw(8) << std::fixed << std::setprecision(2) << elements[i * 4 + j] << " ";
        }
        std::cout << "|\n";
    }
}

// Получение указателя на данные
const float* Matrix4x4::data() const {
    return elements.data();
}
