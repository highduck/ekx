#include "Tween.hpp"
#include "DestroyTimer.hpp"
#include <ek/math.h>

namespace ek {

void handle_end(ecs::EntityApi e, Tween* tween) {
    tween->advanced = nullptr;
    if (tween->destroyEntity) {
        destroyDelay(e);
    }
    else if (!tween->keep) {
        e.remove<Tween>();
    }
}

void update_frame(entity_t e, Tween* tween) {
    if (tween->advanced) {
        const float t = saturate(tween->time / tween->duration);
        tween->advanced(e, t);
    }
}

void Tween::updateAll() {
    for (auto e: ecs::view_backward<Tween>()) {
        auto& tween = e.get<Tween>();
        auto dt = g_time_layers[tween.timer].dt;
        if (tween.delay > 0.0f) {
            tween.delay -= dt;
            continue;
        }
        tween.time += dt;

        update_frame(e.index, &tween);

        if (tween.time >= tween.duration) {
            handle_end(e, &tween);
        }
    }
}

Tween& Tween::reset(ecs::EntityApi e) {
    auto& tween = e.get_or_create<Tween>();
    if (tween.time > 0.0f && tween.time < tween.duration) {
        tween.time = tween.duration;
        update_frame(e.index, &tween);
        tween.advanced = nullptr;
    }
    tween.destroyEntity = false;
    tween.keep = false;
    tween.time = 0.0f;
    return tween;
}

}