#pragma once

#include "Color32.hpp"

namespace ek {

template<typename T>
struct HSVColor {
    T hue;
    T saturation;
    T value;
    T alpha;

    explicit HSVColor(T hue = T(1), T saturation = T(1), T value = T(1), T alpha = T(1)) :
            hue{hue},
            saturation{saturation},
            value{value},
            alpha{alpha} {
    }

    [[nodiscard]]
    argb32_t argb32() const {
        argb32_t hue_color = getHueColor(hue);
        uint8_t r = lerp_channel(hue_color.r, saturation, value);
        uint8_t g = lerp_channel(hue_color.g, saturation, value);
        uint8_t b = lerp_channel(hue_color.b, saturation, value);
        return {r, g, b, uint8_t(alpha * 255.0f)};
    }

    void set_argb32(argb32_t color) {
        auto r = color.r;
        auto g = color.g;
        auto b = color.b;
        auto min = float(std::min(r, std::min(g, b)));
        auto max = float(std::max(r, std::max(g, b)));
        float delta = max - min;
        value = max / 255.0f;
        if (max > 0.0f && delta > 0.0f) {
            saturation = delta / max;
            hue = calc_hue(max, delta, r, g, b);
        } else {
            saturation = 0.0f;
            hue = -1.0f;
        }
        alpha = color.af();
    }

    static argb32_t getHueColor(float hueNormalized) {
        const float t = table_max_ * saturate(hueNormalized);
        const auto index = static_cast<int>(t);
        return lerp(table_[index], table_[index + 1], t - (float)index);
    }

private:
    constexpr static argb32_t table_[7] = {
            0xFF0000_rgb,
            0xFFFF00_rgb,
            0x00FF00_rgb,
            0x00FFFF_rgb,
            0x0000FF_rgb,
            0xFF00FF_rgb,
            0xFF0000_rgb
    };

    constexpr static unsigned table_max_ = 6u;

    static uint8_t lerp_channel(uint8_t value, float x, float y) {
        return unorm8_f32(f32_lerp(0.0f, f32_lerp(255.0f, (float)value, x), y));
    }

    static float calc_hue(float max, float delta, float r, float g, float b) {
        float hue;
        if (r >= max) {
            hue = (g - b) / delta;
        } else if (g >= max) {
            hue = 2.0f + (b - r) / delta;
        } else {
            hue = 4.0f + (r - g) / delta;
        }

        hue /= (float)table_max_;
        if (hue < 0.0f) {
            hue += 1.0f;
        }

        return hue;
    }
};

using HSVColor4f = HSVColor<float>;

}


