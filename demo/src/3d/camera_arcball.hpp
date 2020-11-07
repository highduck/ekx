#pragma once

#include <ek/math/vec.hpp>

namespace ek {

struct camera_arc_ball {
    float3 center{};
    float distance = 200.0f;
};

void update_camera_arc_ball(float dt);

}

