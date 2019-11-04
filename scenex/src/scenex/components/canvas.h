#pragma once

#include <scenex/config.h>

namespace scenex {

struct canvas_t {
    rect_f resolution{0.0f, 0.0f, 1.0f, 1.0f};
    float scale = 1.0f;
    bool landscape = false;

    canvas_t() = default;

    canvas_t(float width, float height)
            : resolution{0.0f, 0.0f, width, height},
              landscape{width > height} {

    }
};

}