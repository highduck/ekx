#pragma once

namespace ek {

template<unsigned Cols, unsigned Rows, typename T>
struct Matrix {
    T data_[Rows][Cols];

    static_assert(sizeof(Matrix) == sizeof(T) * Rows * Cols, "struct has extra padding");
};

template<typename T>
struct Matrix<4, 4, T>;

template<typename T>
struct Matrix<2, 3, T>;

template<typename T>
struct Matrix<2, 2, T>;

template<typename T>
struct Matrix<3, 3, T>;

template<typename T>
using Matrix4 = Matrix<4, 4, T>;

template<typename T>
using Matrix3x2 = Matrix<3, 2, T>;

using Matrix3x2f = Matrix<3, 2, float>;
using Matrix4f = Matrix<4, 4, float>;
using Matrix3f = Matrix<3, 3, float>;

}