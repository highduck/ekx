#pragma once

#include <cstdint>
#include "vec.hpp"
//#include "packed_color.h"

namespace ek {

template<typename T>
struct color_transform_t {

    using color_type = vec_t<4, T>;
    color_type multiplier;
    color_type offset;

    explicit color_transform_t(T red_multiplier,
                               T green_multiplier = 1,
                               T blue_multiplier = 1,
                               T alpha_multiplier = 1,
                               T red_offset = 0,
                               T green_offset = 0,
                               T blue_offset = 0,
                               T alpha_offset = 0) noexcept
            : multiplier{red_multiplier, green_multiplier, blue_multiplier, alpha_multiplier},
              offset{red_offset, green_offset, blue_offset, alpha_offset} {
    }

    explicit color_transform_t(const color_type& multiplier_,
                               const color_type& offset_ = color_type::zero) noexcept
            : multiplier{multiplier_},
              offset{offset_} {

    }

    inline color_transform_t() noexcept
            : multiplier{color_type::one},
              offset{color_type::zero} {

    }

    /**
     *
     * @param color - RGB part
     * @param intensity - float 0...1
     */
    void tint(uint32_t color, float intensity) {
        multiplier.x = T(1) - intensity;
        multiplier.y = T(1) - intensity;
        multiplier.z = T(1) - intensity;
        offset.x = float((color >> 16u) & 0xFFu) * intensity / 255.0f;
        offset.y = float((color >> 8u) & 0xFFu) * intensity / 255.0f;
        offset.z = float(color & 0xFFu) * intensity / 255.0f;
    }

    inline color_transform_t<T> operator*(const color_transform_t<T>& right) const {
        return color_transform_t<T>(
                multiplier * right.multiplier,
                multiplier * right.offset + offset
        );
    }

    inline color_type transform(const color_type& color) const {
        return color * multiplier + offset;
    }
};

using color_transform_f = color_transform_t<float>;

}
