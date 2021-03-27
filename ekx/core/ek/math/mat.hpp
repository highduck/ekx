#pragma once

#include "mat2x2.hpp"
#include "mat3x2.hpp"
#include "mat3x3.hpp"
#include "mat4x4.hpp"

namespace ek {

template<typename T>
using mat4_t = matrix_t<4, 4, T>;

template<typename T>
using mat3x2_t = matrix_t<3, 2, T>;

using matrix_2d = matrix_t<3, 2, float>;
using mat4f = matrix_t<4, 4, float>;
using mat3f = matrix_t<3, 3, float>;

}