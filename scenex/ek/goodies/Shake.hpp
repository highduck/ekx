#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/timers.hpp>

namespace ek {

struct Shake {
    float time = 0.0f;
    float time_total = 1.0f;
    float strength = 16.0f;
    TimeLayer timer;

    static void updateAll();
    static void add(ecs::entity e, float time = 2.0f, float strength = 16.0f);
};

}


