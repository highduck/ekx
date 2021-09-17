#pragma once

#include <cstdint>
#include "vec.hpp"
//#include "packed_color.h"

namespace ek {

template<typename T>
struct color_transform_t {

    using color_type = vec_t<4, T>;
    color_type scale;
    color_type offset;

    explicit color_transform_t(T red_scale,
                               T green_scale = 1,
                               T blue_scale = 1,
                               T alpha_scale = 1,
                               T red_offset = 0,
                               T green_offset = 0,
                               T blue_offset = 0,
                               T alpha_offset = 0) noexcept
            : scale{red_scale, green_scale, blue_scale, alpha_scale},
              offset{red_offset, green_offset, blue_offset, alpha_offset} {
    }

    explicit color_transform_t(const color_type& multiplier_,
                               const color_type& offset_ = color_type::zero) noexcept
            : scale{multiplier_},
              offset{offset_} {

    }

    inline color_transform_t() noexcept
            : scale{color_type::one},
              offset{color_type::zero} {

    }

    /**
     *
     * @param color - RGB part
     * @param intensity - float 0...1
     */
    void tint(uint32_t color, float intensity) {
        scale.x = T(1) - intensity;
        scale.y = T(1) - intensity;
        scale.z = T(1) - intensity;
        offset.x = float((color >> 16u) & 0xFFu) * intensity / 255.0f;
        offset.y = float((color >> 8u) & 0xFFu) * intensity / 255.0f;
        offset.z = float(color & 0xFFu) * intensity / 255.0f;
    }

    inline color_transform_t<T> operator*(const color_transform_t<T>& right) const {
        return color_transform_t<T>(
                scale * right.scale,
                scale * right.offset + offset
        );
    }

    inline color_transform_t<T> operator*(T v) const {
        return color_transform_t<T>(
                scale * v,
                offset * v
        );
    }

    inline color_transform_t<T> operator-(const color_transform_t<T>& right) const {
        return color_transform_t<T>(
                scale - right.scale,
                offset - right.offset
        );
    }

    inline color_transform_t<T> operator+(const color_transform_t<T>& right) const {
        return color_transform_t<T>(
                scale + right.scale,
                offset + right.offset
        );
    }

    inline color_type transform(const color_type& color) const {
        return color * scale + offset;
    }

};

template<typename T>
inline color_transform_t<T> operator*(T scalar, const color_transform_t<T>& v) {
    return color_transform_t{scalar * v.scale, scalar * v.offset};
}

template<typename T>
inline color_transform_t<T> lerp(const color_transform_t<T>& begin, const color_transform_t<T>& end, T t) {
    return (T{1} - t) * begin + t * end;
}

using color_transform_f = color_transform_t<float>;

}
