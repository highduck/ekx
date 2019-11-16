#pragma once

#include <ek/math/vec.hpp>

namespace ek {

struct position_t {
    float2 position;

    position_t() = default;

    position_t(float x, float y)
            : position{x, y} {

    }
};

}