#pragma once

#include <ek/math/color_transform.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/xfl/types.hpp>

namespace ek::xfl {

class transform_model {
public:
    matrix_2d matrix{};
    color_transform_f color{};
    blend_mode_t blend_mode{blend_mode_t::normal};

    transform_model() = default;

    transform_model(const matrix_2d& matrix_, const color_transform_f& color_, blend_mode_t blend_mode_ = blend_mode_t::last)
            : matrix{matrix_},
              color{color_},
              blend_mode{blend_mode_} {

    }

    inline transform_model operator*(const transform_model& right) const {
        return {
            matrix * right.matrix,
                color * right.color,
                right.blend_mode == blend_mode_t::last ? blend_mode : right.blend_mode
        };
    }
};

}


