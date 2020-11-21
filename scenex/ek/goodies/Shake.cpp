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

        e.get<Transform2D>().position = offset * r * state.strength;

        if (state.time <= 0.0f) {
            e.remove<Shake>();
        }
    }
}

void Shake::add(ecs::entity e, float time, float strength) {
    auto& state = e.get_or_create<Shake>();
    state.time = 0.0f;
    state.time_total = time;
    state.strength = strength;
}

}