#include "shake_system.hpp"

#include <ek/scenex/components/transform_2d.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/rand.hpp>
#include <ek/scenex/systems/game_time.hpp>

namespace ek {

void update_shake() {

    for (auto e: ecs::rview<shake_state_t>()) {
        auto& state = e.get<shake_state_t>();
        auto dt = get_delta_time(e);
        state.time += dt;

        float r = std::max(0.0f, 1.0f - state.time / state.time_total);
        float2 offset{
                rand_fx.random(-1.0f, 1.0f),
                rand_fx.random(-1.0f, 1.0f)
        };

        e.get<transform_2d>().position = offset * r * state.strength;

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