#pragma once

namespace ek {

template<unsigned Cols, unsigned Rows, typename T>
struct matrix_t {
    T data_[Rows][Cols];

    static_assert(sizeof(matrix_t) == sizeof(T) * Rows * Cols, "struct has extra padding");
};

template<typename T>
struct matrix_t<4, 4, T>;

template<typename T>
struct matrix_t<2, 3, T>;

template<typename T>
struct matrix_t<2, 2, T>;

template<typename T>
struct matrix_t<3, 3, T>;

template<typename T>
using mat4_t = matrix_t<4, 4, T>;

template<typename T>
using mat3x2_t = matrix_t<3, 2, T>;

using matrix_2d = matrix_t<3, 2, float>;
using mat4f = matrix_t<4, 4, float>;
using mat3f = matrix_t<3, 3, float>;

}