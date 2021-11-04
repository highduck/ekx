#include "Shake.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/rand.hpp>

namespace ek {

void Shake::updateAll() {

    for (auto e: ecs::view_backward<Shake>()) {
        auto& state = e.get<Shake>();
        auto dt = state.timer->dt;
        state.time += dt;

        float r = std::max(0.0f, 1.0f - state.time / state.time_total);
        float2 offset{
                rand_fx.random(-1.0f, 1.0f),
                rand_fx.random(-1.0f, 1.0f)
        };

        e.get<Transform2D>().setPosition(offset * r * state.strength);

        if (state.time <= 0.0f) {
            e.remove<Shake>();
        }
    }
}

void Shake::add(ecs::EntityApi e, float time, float strength) {
    auto& state = e.get_or_create<Shake>();
    state.time = 0.0f;
    state.time_total = time;
    state.strength = strength;
}

inline float2 randomF2(float min, float max) {
    return float2{random(min, max), random(min, max)};
}

void Shaker::updateAll() {
    for (auto e: ecs::view<Shaker>()) {
        auto& data = e.get<Shaker>();
        const auto dt = data.timer->dt;
        data.state = math::reach(data.state, 0.0f, dt);
        const auto r = math::integrateExp(0.9f, dt);

        const auto posTarget = randomF2(0.0f, 1.0f) * data.maxOffset * data.state;
        const auto rotTarget = random(-0.5f, 0.5f) * data.maxRotation * data.state;
        const auto scaleTarget = float2::one + randomF2(-0.5f, 0.5f) * data.maxScale * data.state;
        auto& pos = e.get<Transform2D>();
        pos.setRotation(math::lerp(pos.getRotation(), rotTarget, r));
        pos.setScale(lerp(pos.getScale(), scaleTarget, r));
        pos.setPosition(lerp(pos.getPosition(), posTarget, r));
    }
}

}