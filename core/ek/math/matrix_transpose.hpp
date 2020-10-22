#pragma once

#include "mat3x3.hpp"

namespace ek{

template <typename T>
matrix_t<3, 3, T> transpose(const matrix_t<3, 3, T>& m) {
    matrix_t<3, 3, T> result;
    result(0, 0) = m(0, 0);
    result(0, 1) = m(1, 0);
    result(0, 2) = m(2, 0);

    result(1, 0) = m(0, 1);
    result(1, 1) = m(1, 1);
    result(1, 2) = m(2, 1);

    result(2, 0) = m(0, 2);
    result(2, 1) = m(1, 2);
    result(2, 2) = m(2, 2);
    return result;
}
}