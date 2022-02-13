#include "Shake.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/rnd.h>

namespace ek {
//
//void Shake::updateAll() {
//
//    for (auto e: ecs::view_backward<Shake>()) {
//        auto& state = ecs::get<Shake>(e);
//        auto dt = g_time_layers[state.timer].dt;
//        state.time += dt;
//
//        float r = fmaxf(0.0f, 1.0f - state.time / state.time_total);
//        vec2_t offset = vec2(
//                random_range_f(-1.0f, 1.0f),
//                random_range_f(-1.0f, 1.0f)
//        );
//
//        ecs::get<Transform2D>(e).set_position(offset * r * state.strength);
//
//        if (state.time <= 0.0f) {
//            ecs::remove<Shake>(e);
//        }
//    }
//}
//
//void Shake::add(ecs::Entity e, float time, float strength) {
//    auto& state = ecs::add<Shake>(e);
//    state.time = 0.0f;
//    state.time_total = time;
//    state.strength = strength;
//}

inline vec2_t randomF2(float min, float max) {
    return vec2(random_range_f(min, max), random_range_f(min, max));
}

void Shaker::updateAll() {
    for (auto e: ecs::view<Shaker>()) {
        auto& data = ecs::get<Shaker>(e);
        const auto dt = g_time_layers[data.timer].dt;
        data.state = reach(data.state, 0.0f, dt);
        const auto r = integrate_exp(0.9f, dt, 0);

        const auto posTarget = randomF2(0.0f, 1.0f) * data.maxOffset * data.state;
        const auto rotTarget = random_range_f(-0.5f, 0.5f) * data.maxRotation * data.state;
        const auto scaleTarget = vec2(1, 1) + randomF2(-0.5f, 0.5f) * data.maxScale * data.state;
        auto& pos = ecs::get<Transform2D>(e);
        pos.set_rotation(lerp_f32(pos.getRotation(), rotTarget, r));
        pos.set_scale(lerp_vec2(pos.getScale(), scaleTarget, r));
        pos.set_position(lerp_vec2(pos.getPosition(), posTarget, r));
    }
}

}