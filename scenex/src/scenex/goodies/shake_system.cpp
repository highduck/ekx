#include "shake_system.h"

#include <scenex/components/transform_2d.h>
#include <ek/math/vec.hpp>
#include <ek/math/rand.hpp>
#include <scenex/systems/game_time.h>

using namespace scenex;

namespace scenex {

void update_shake() {
    using ek::rand_fx;

    for (auto e: ecs::rview<shake_state_t>()) {
        auto& state = ecs::get<shake_state_t>(e);
        auto dt = get_delta_time(e);
        state.time += dt;

        float r = std::max(0.0f, 1.0f - state.time / state.time_total);
        float2 offset{
                rand_fx.random(-1.0f, 1.0f),
                rand_fx.random(-1.0f, 1.0f)
        };

        ecs::get<transform_2d>(e).matrix.position(offset * r * state.strength);

        if (state.time <= 0.0f) {
            ecs::remove<shake_state_t>(e);
        }
    }
}

void start_shake(ecs::entity e, float time, float strength) {
    auto& state = ecs::get_or_create<shake_state_t>(e);
    state.time = 0.0f;
    state.time_total = time;
    state.strength = strength;
}

}