#pragma once

#include "Color32.hpp"

namespace ek {

struct HSVColor {
    float hue;
    float saturation;
    float value;
    float alpha;

    explicit HSVColor(float hue = 1.0f, float saturation = 1.0f, float value = 1.0f, float alpha = 1.0f) :
            hue{hue},
            saturation{saturation},
            value{value},
            alpha{alpha} {
    }

    [[nodiscard]]
    rgba_t rgba() const {
        const rgba_t hue_color = getHueColor(hue);
        rgba_t result;
        result.r = lerp_channel(hue_color.r, saturation, value);
        result.g = lerp_channel(hue_color.g, saturation, value);
        result.b = lerp_channel(hue_color.b, saturation, value);
        result.a = (uint8_t)(alpha * 255.0f);
        return result;
    }

    void set_argb32(rgba_t color) {
        const uint8_t r = color.r;
        const uint8_t g = color.g;
        const uint8_t b = color.b;
        const float min = (float)(MIN(r, MIN(g, b)));
        const float max = (float)(MAX(r, MAX(g, b)));
        const float delta = max - min;
        value = max / 255.0f;
        if (max > 0.0f && delta > 0.0f) {
            saturation = delta / max;
            hue = calc_hue(max, delta, r, g, b);
        } else {
            saturation = 0.0f;
            hue = -1.0f;
        }
        alpha = (float)color.a / 255.0f;
    }

    static rgba_t getHueColor(float hueNormalized) {
        const float t = table_max_ * saturate(hueNormalized);
        const int index = (int)t;
        return rgba_lerp(table_[index], table_[index + 1], t - (float)index);
    }

private:
    constexpr static rgba_t table_[7] = {
            RGB(0xFF0000),
            RGB(0xFFFF00),
            RGB(0x00FF00),
            RGB(0x00FFFF),
            RGB(0x0000FF),
            RGB(0xFF00FF),
            RGB(0xFF0000),
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

}


