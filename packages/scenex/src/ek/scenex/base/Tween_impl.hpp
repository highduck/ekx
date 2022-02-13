#include "Tween.hpp"
#include "DestroyTimer.hpp"
#include <ek/math.h>

namespace ek {

void handle_end(entity_t e, Tween* tween) {
    if (tween->destroy_entity) {
        destroy_later(e);
    } else if (!tween->keep) {
        ecs::remove<Tween>(e);
    }
}

void on_tween_completed(entity_t e, Tween* tween) {
    if(tween->completed) {
        tween->completed(e);
        tween->completed = nullptr;
    }
}


void update_frame(entity_t e, Tween* tween) {
    if (tween->advanced) {
        const float t = saturate(tween->time / tween->duration);
        tween->advanced(e, t);
    }
}

void update_tweens() {
    for (auto e: ecs::view_backward<Tween>()) {
        Tween* tween = (Tween* )get_component(ecs::type<Tween>(), e);
        auto dt = g_time_layers[tween->timer].dt;
        if (tween->delay > 0.0f) {
            tween->delay -= dt;
            continue;
        }
        tween->time += dt;
        bool completed = false;
        if (tween->time >= tween->duration) {
            tween->time = tween->duration;
            completed = true;
        }
        update_frame(e, tween);

        if (completed) {
            tween->advanced = nullptr;
            on_tween_completed(e, tween);
            handle_end(e, tween);
        }
    }
}

Tween& Tween::reset(entity_t e) {
    auto& tween = ecs::add<Tween>(e);
    if (tween.time > 0.0f && tween.time < tween.duration) {
        tween.time = tween.duration;
        update_frame(e, &tween);
        tween.advanced = nullptr;
        on_tween_completed(e, &tween);
    }
    tween.destroy_entity = false;
    tween.keep = false;
    tween.time = 0.0f;
    return tween;
}

}