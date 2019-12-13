#pragma once

#include <ek/math/mat3x2.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>
#include <ek/math/packed_color.hpp>

namespace ek {

struct transform_2d {

    mutable matrix_2d matrix{};
    float2 scale = float2::one;
    float2 skew = float2::zero;
    float2 origin = float2::zero;
    rect_f rect = rect_f::zero;

    matrix_2d* user_matrix = nullptr;

    argb32_t color_multiplier = argb32_t::one;
    argb32_t color_offset = argb32_t::zero;

    void rotation(float value) {
        skew = {value, value};
    }

    [[nodiscard]] float rotation() const {
        return skew.x == skew.y ? skew.y : 0.0f;
    }

    void rotate(float value) {
        skew.x += value;
        skew.y += value;
    }

    inline void set_alpha(float alpha) {
        color_multiplier.af(alpha);
    }

    [[nodiscard]] inline float get_alpha() const {
        return color_multiplier.af();
    }

    inline void set_additive(float alpha) {
        color_offset.af(alpha);
    }

    [[nodiscard]] inline float get_additive() const {
        return color_offset.af();
    }
};


void begin_transform(const transform_2d& transform);

void end_transform();

}


