#pragma once

#include <ecxx/ecxx.hpp>
#include "TimeLayer.hpp"

namespace ek {

struct DestroyTimer {
    float delay = 0.0f;
    TimeLayer timer{};

    static void updateAll();
};

ECX_TYPE(8, DestroyTimer);

void destroyDelay(ecs::EntityApi e, float delay = 0.0f, TimeLayer timer = {});

void destroyChildrenDelay(ecs::EntityApi e, float delay = 0.0f, TimeLayer timer = {});

}


