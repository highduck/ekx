#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/util/Signal.hpp>
#include "TimeLayer.hpp"

namespace ek {

struct Tween {
    float delay = 0.0f;
    float time = 0.0f;
    float duration = 1.0f;
    Signal<float> advanced;
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

ECX_TYPE(6, Tween);

}