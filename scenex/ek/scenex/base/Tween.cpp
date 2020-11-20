#include "Tween.hpp"
#include <ek/scenex/utility/destroy_delay.hpp>
#include <ek/math/common.hpp>

namespace ek {

void handle_end(ecs::entity e) {
    auto& tween = ecs::get<Tween>(e);
    tween.advanced.clear();
    if (tween.destroyEntity) {
        destroy_delay(e);
    } else if (tween.auto_destroy) {
        ecs::remove<Tween>(e);
    }
}

void update_frame(Tween& tween) {
    const float t = math::clamp(tween.time / tween.duration, 0.0f, 1.0f);
    tween.advanced(t);
}

void Tween::updateAll() {
    for (auto e : ecs::rview<Tween>()) {
        auto& tween = ecs::get<Tween>(e);
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

Tween& Tween::reset(ecs::entity e) {
    auto& tween = ecs::get_or_create<Tween>(e);
    if (tween.time > 0.0f && tween.time < tween.duration) {
        tween.time = tween.duration;
        update_frame(tween);
        tween.advanced.clear();
    }
    tween.time = 0.0f;
    return tween;
}

}