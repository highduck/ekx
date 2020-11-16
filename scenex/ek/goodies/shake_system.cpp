#include "shake_system.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/rand.hpp>

namespace ek {

void update_shake() {

    for (auto e: ecs::rview<shake_state_t>()) {
        auto& state = e.get<shake_state_t>();
        auto dt = state.timer->dt;
        state.time += dt;

        float r = std::max(0.0f, 1.0f - state.time / state.time_total);
        float2 offset{
                rand_fx.random(-1.0f, 1.0f),
                rand_fx.random(-1.0f, 1.0f)
        };

        e.get<Transform2D>().position = offset * r * state.strength;

        if (state.time <= 0.0f) {
            e.remove<shake_state_t>();
        }
    }
}

void start_shake(ecs::entity e, float time, float strength) {
    auto& state = e.get_or_create<shake_state_t>();
    state.time = 0.0f;
    state.time_total = time;
    state.strength = strength;
}

}