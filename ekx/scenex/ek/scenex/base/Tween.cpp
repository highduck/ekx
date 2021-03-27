#include "Tween.hpp"
#include <ek/scenex/base/DestroyTimer.hpp>
#include <ek/math/common.hpp>

namespace ek {

void handle_end(ecs::entity e) {
    auto& tween = e.get<Tween>();
    tween.advanced.clear();
    if (tween.destroyEntity) {
        destroyDelay(e);
    } else if (tween.auto_destroy) {
        e.remove<Tween>();
    }
}

void update_frame(Tween& tween) {
    const float t = math::clamp(tween.time / tween.duration, 0.0f, 1.0f);
    tween.advanced(t);
}

void Tween::updateAll() {
    for (auto e : ecs::view_backward<Tween>()) {
        auto& tween = e.get<Tween>();
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
    auto& tween = e.get_or_create<Tween>();
    if (tween.time > 0.0f && tween.time < tween.duration) {
        tween.time = tween.duration;
        update_frame(tween);
        tween.advanced.clear();
    }
    tween.time = 0.0f;
    return tween;
}

}