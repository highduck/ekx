#pragma once

#include <ek/math/box.hpp>

namespace ek {

struct Canvas {
    float2 resolution = float2::one;
    float scale = 1.0f;
    bool landscape = false;

    Canvas() = default;

    Canvas(float width, float height) : resolution{width, height},
                                        landscape{width > height} {

    }

    static void updateAll();
};

}