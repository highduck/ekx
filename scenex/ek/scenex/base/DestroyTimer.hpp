#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/timers.hpp>

namespace ek {

struct DestroyTimer {
    float delay = 0.0f;
    TimeLayer timer{};

    static void updateAll();
};

void destroy_delay(ecs::entity e, float delay = 0.0f, TimeLayer timer = {});

void destroy_children_delay(ecs::entity e, float delay = 0.0f, TimeLayer timer = {});

}


