#include "Shake.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/math/Vec.hpp>
#include <ek/math/Random.hpp>

namespace ek {

void Shake::updateAll() {

    for (auto e: ecs::view_backward<Shake>()) {
        auto& state = e.get<Shake>();
        auto dt = state.timer->dt;
        state.time += dt;

        float r = std::max(0.0f, 1.0f - state.time / state.time_total);
        Vec2f offset{
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

inline Vec2f randomF2(float min, float max) {
    return Vec2f{random(min, max), random(min, max)};
}

void Shaker::updateAll() {
    for (auto e: ecs::view<Shaker>()) {
        auto& data = e.get<Shaker>();
        const auto dt = data.timer->dt;
        data.state = Math::reach(data.state, 0.0f, dt);
        const auto r = Math::integrateExp(0.9f, dt);

        const auto posTarget = randomF2(0.0f, 1.0f) * data.maxOffset * data.state;
        const auto rotTarget = random(-0.5f, 0.5f) * data.maxRotation * data.state;
        const auto scaleTarget = Vec2f::one + randomF2(-0.5f, 0.5f) * data.maxScale * data.state;
        auto& pos = e.get<Transform2D>();
        pos.setRotation(Math::lerp(pos.getRotation(), rotTarget, r));
        pos.setScale(lerp(pos.getScale(), scaleTarget, r));
        pos.setPosition(lerp(pos.getPosition(), posTarget, r));
    }
}

}