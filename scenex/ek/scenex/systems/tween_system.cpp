#include "tween_system.hpp"

#include <ek/scenex/components/tween.hpp>
#include <ek/math/common.hpp>
#include <ek/scenex/utility/destroy_delay.hpp>

namespace ek {

void handle_end(ecs::entity e) {
    auto& tween = ecs::get<tween_t>(e);
    tween.advanced.clear();
    if (tween.destroyEntity) {
        destroy_delay(e);
    } else if (tween.auto_destroy) {
        ecs::remove<tween_t>(e);
    }
}

void update_frame(tween_t& tween) {
    const float t = math::clamp(tween.time / tween.duration, 0.0f, 1.0f);
    tween.advanced(t);
}

void update_tweens() {
    for (auto e : ecs::rview<tween_t>()) {
        auto& tween = ecs::get<tween_t>(e);
        auto dt = tween.timer->dt;
        if (tween.delay > 0.0f) {
            tween.delay -= dt;
            continue;
        }
        tween.time += dt;

        update_frame(tween);

        if (tween.time >= tween.duration) {
            handle_end(e);
        }
    }
}

tween_t& reset_tween(ecs::entity e) {
    auto& tween = ecs::get_or_create<tween_t>(e);
    if (tween.time > 0.0f && tween.time < tween.duration) {
        tween.time = tween.duration;
        update_frame(tween);
        tween.advanced.clear();
    }
    tween.time = 0.0f;
    return tween;
}

}