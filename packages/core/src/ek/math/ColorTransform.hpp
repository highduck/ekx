#pragma once

#include <cstdint>
#include <ek/math.h>

namespace ek {

struct ColorTransformF {
    vec4_t scale;
    vec4_t offset;

    explicit ColorTransformF(float red_scale,
                             float green_scale = 1,
                             float blue_scale = 1,
                             float alpha_scale = 1,
                             float red_offset = 0,
                             float green_offset = 0,
                             float blue_offset = 0,
                             float alpha_offset = 0) noexcept
            : scale{{red_scale, green_scale, blue_scale, alpha_scale}},
              offset{{red_offset, green_offset, blue_offset, alpha_offset}} {
    }

    explicit ColorTransformF(const vec4_t multiplier_,
                             const vec4_t offset_ = {}) noexcept
            : scale{multiplier_},
              offset{offset_} {

    }

    inline ColorTransformF() noexcept
            : scale{{1, 1, 1, 1}},
              offset{{0, 0, 0, 0}} {

    }

    /**
     *
     * @param color - RGB part
     * @param intensity - float 0...1
     */
    void tint(uint32_t color, float intensity) {
        scale.x = 1.0f - intensity;
        scale.y = 1.0f - intensity;
        scale.z = 1.0f - intensity;
        offset.x = (float) ((color >> 16u) & 0xFFu) * intensity / 255.0f;
        offset.y = (float) ((color >> 8u) & 0xFFu) * intensity / 255.0f;
        offset.z = (float) (color & 0xFFu) * intensity / 255.0f;
    }

    inline ColorTransformF operator*(const ColorTransformF& right) const {
        return ColorTransformF(
                scale * right.scale,
                scale * right.offset + offset
        );
    }

    inline ColorTransformF operator*(float v) const {
        return ColorTransformF(
                scale * v,
                offset * v
        );
    }

    inline ColorTransformF operator-(const ColorTransformF& right) const {
        return ColorTransformF(
                scale - right.scale,
                offset - right.offset
        );
    }

    inline ColorTransformF operator+(const ColorTransformF& right) const {
        return ColorTransformF(
                scale + right.scale,
                offset + right.offset
        );
    }

    inline vec4_t transform(const vec4_t color) const {
        return color * scale + offset;
    }

};

inline ColorTransformF operator*(float scalar, const ColorTransformF& v) {
    return ColorTransformF{v.scale * scalar, v.offset * scalar};
}

inline ColorTransformF lerp_color_mod_f(const ColorTransformF& begin, const ColorTransformF& end, float t) {
    return (1.0f - t) * begin + t * end;
}

}
