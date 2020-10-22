#pragma once

#include <ek/math/packed_color.hpp>
#include <ek/util/timer.hpp>
#include <ek/util/locator.hpp>
#include <algorithm>

namespace ek::piko {

static argb32_t palette[] = {
        0x000000_argb, //black
        0x1D2B53_argb, // dark-blue
        0x7E2553_argb, // dark-purple
        0x008751_argb, // dark-green
        0xAB5236_argb, // brown
        0x5F574F_argb, // dark-gray
        0xC2C3C7_argb, // light-gray
        0xFFF1E8_argb, // white
        0xFF004D_argb, // red
        0xFFA300_argb, // orange
        0xFFEC27_argb, // yellow
        0x00E436_argb, // green
        0x29ADFF_argb, // blue
        0x83769C_argb, // indigo
        0xFF77A8_argb, // pink
        0xFFCCAA_argb, // peach
//			0x000000, // reset cycle
};

inline static argb32_t colorf(float index) {
    if (index < 0.0f) index = 0.0f;
    int i = (int) index;
    int e = i + 1;
    auto* colors = palette;
//    int count = sizeof(kPalette) / sizeof(ARGB32);
    int count = 16;
    if (e >= count) e = count - 1;
    return lerp(colors[i], colors[e], index - i) | 0xFF000000;
}

inline static float time() {
    return static_cast<float>(ek::clock::now());
}

inline static float mid(float x, float y, float z = 0.0f) {
    if (x > y) {
        std::swap(x, y);
    }
    return std::max(x, std::min(y, z));
}

inline static float sqr(float a) {
    return a * a;
}

inline static int sgn(float a) {
    return a >= 0.0f ? 1 : -1;
}

inline static float sinu(float unit) {
    return -sinf(unit * math::pi2);
}

inline static float cosu(float unit) {
    return cosf(unit * math::pi2);
}

}