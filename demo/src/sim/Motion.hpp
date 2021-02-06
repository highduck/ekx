#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>

namespace ek::sim {

struct motion_t {
    float2 velocity;
};

struct attractor_t {
    float radius = 100.0f;
    float force = 10'000.0f;
};

void update_motion_system(float dt);

}


