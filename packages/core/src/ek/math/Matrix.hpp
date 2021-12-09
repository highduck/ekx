#pragma once

#include "Matrix2.hpp"
#include "Matrix3x2.hpp"
#include "Matrix3.hpp"
#include "Matrix4.hpp"

namespace ek {

template<typename T>
using Matrix4 = Matrix<4, 4, T>;

template<typename T>
using Matrix3x2 = Matrix<3, 2, T>;

using Matrix3x2f = Matrix<3, 2, float>;
using Matrix4f = Matrix<4, 4, float>;
using Matrix3f = Matrix<3, 3, float>;

}