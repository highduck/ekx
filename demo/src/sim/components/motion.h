#pragma once

#include <ek/math/vec.hpp>

namespace ek {

struct motion_t {
    float2 velocity;

    motion_t() = default;

    motion_t(float vx, float vy)
            : velocity{vx, vy} {

    }
};

}