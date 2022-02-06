#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/util/Signal.hpp>
#include <ekx/app/time_layers.h>

namespace ek {

struct Tween {
    void (*advanced)(entity_t e, float t) = nullptr;
    int data[4] = {0};
    float delay = 0.0f;
    float time = 0.0f;
    float duration = 0.0f;
    TimeLayer timer = 0;
    bool keep = false;
    bool destroyEntity = false;

    static void updateAll();

    static Tween& reset(ecs::EntityApi e);

    void setOptions(float duration_ = 1.0f, float delay_ = 0.0f) {
        duration = duration_;
        delay = delay_;
    }
};

}
