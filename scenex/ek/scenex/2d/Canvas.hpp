#pragma once

#include <ek/math/box.hpp>

namespace ek {

struct Canvas {
    rect_f resolution{0.0f, 0.0f, 1.0f, 1.0f};
    float scale = 1.0f;
    bool landscape = false;

    Canvas() = default;

    Canvas(float width, float height)
            : resolution{0.0f, 0.0f, width, height},
              landscape{width > height} {

    }

    static void updateAll();
};

}