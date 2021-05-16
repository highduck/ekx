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

    void setOptions(float duration_ = 1.0f, float delay_ = 0.0f) {
        duration = duration_;
        delay = delay_;
    }
};

EK_DECLARE_TYPE(Tween);

}