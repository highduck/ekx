#pragma once

#include <ecxx/ecxx_fwd.hpp>

struct fireworks_comp {
    float timer_ = 1.0f;
    bool enabled = false;
};

void setup_fireworks(ecs::EntityApi e);

void update_fireworks(float dt);

void reset_fireworks(ecs::EntityApi e);
