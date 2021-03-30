#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/timers.hpp>

namespace ek {

struct DestroyTimer {
    float delay = 0.0f;
    TimeLayer timer{};

    static void updateAll();
};

void destroyDelay(ecs::EntityApi e, float delay = 0.0f, TimeLayer timer = {});

void destroyChildrenDelay(ecs::EntityApi e, float delay = 0.0f, TimeLayer timer = {});

}


