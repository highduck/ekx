#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/util/signals.hpp>
#include <ek/timers.hpp>

namespace ek {

struct Tween {
    float delay = 0.0f;
    float time = 0.0f;
    float duration = 1.0f;
    signal_t<float> advanced;
    bool auto_destroy = true;
    bool destroyEntity = false;
    TimeLayer timer;

    static void updateAll();

    static Tween& reset(ecs::EntityApi e);
};

}