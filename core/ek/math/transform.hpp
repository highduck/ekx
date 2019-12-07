#pragma once

#include "mat3x2.hpp"
#include "color_transform.hpp"

namespace ek {

template<typename T>
struct transform_t {

    using matrix_type = mat3x2_t<T>;
    using color_type = color_transform_t<T>;

    matrix_type matrix;
    color_type color;

    inline transform_t() noexcept
            : matrix{},
              color{} {
    }

    transform_t(const matrix_type& matrix_, const color_type& color_) noexcept
            : matrix{matrix_},
              color{color_} {

    }

    transform_t multiply(const matrix_type& matrix_, const color_type& color_) const {
        return {matrix * matrix_, color * color_};
    }

    inline transform_t operator*(const transform_t& transform) const {
        return multiply(transform.matrix, transform.color);
    }
};

}