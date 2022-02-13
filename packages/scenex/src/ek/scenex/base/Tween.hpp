#pragma once

#include <ecx/ecx.hpp>
#include <ek/util/Signal.hpp>
#include <ekx/app/time_layers.h>

namespace ek {

struct Tween {
    void (*advanced)(entity_t e, float t) = nullptr;
    void (*completed)(entity_t e) = nullptr;
    int data[4] = {0};
    float delay = 0.0f;
    float time = 0.0f;
    float duration = 0.0f;
    TimeLayer timer = 0;
    bool keep = false;
    // destroy entity on complete
    bool destroy_entity = false;

    static Tween& reset(entity_t e);

    void setOptions(float duration_ = 1.0f, float delay_ = 0.0f) {
        duration = duration_;
        delay = delay_;
    }
};

void update_tweens();

}
