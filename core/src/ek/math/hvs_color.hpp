#pragma once

#include "packed_color.hpp"
#include <algorithm>

namespace ek {

struct hvs_color_t {
    float hue;
    float value;
    float saturation;
    float alpha;

    explicit hvs_color_t(float hue = 1.0f, float value = 1.0f, float saturation = 1.0f, float alpha = 1.0f) :
            hue{hue},
            value{value},
            saturation{saturation},
            alpha{alpha} {
    }

    argb32_t argb32() const {
        argb32_t hue_color = get_hue_color(hue);
        uint8_t r = lerp_channel(hue_color.r, saturation, value);
        uint8_t g = lerp_channel(hue_color.g, saturation, value);
        uint8_t b = lerp_channel(hue_color.b, saturation, value);
        return argb32_t(r, g, b, uint8_t(alpha * 255.0f));
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

    static argb32_t get_hue_color(float hue_normalized) {
        const float t = table_max_ * math::clamp(hue_normalized);
        const auto index = static_cast<unsigned>(t);
        return lerp(table_[index], table_[index + 1], t - index);
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
        return uint8_t(math::lerp(0.0f, math::lerp(255.0f, float(value), x), y));
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

        hue /= float(table_max_);
        if (hue < 0.0f) {
            hue += 1.0f;
        }

        return hue;
    }
};

}


