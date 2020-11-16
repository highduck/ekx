#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/game_time.hpp>

namespace ek {

struct shake_state_t {
    float time = 0.0f;
    float time_total = 1.0f;
    float strength = 16.0f;
    TimeLayer timer;
};

void update_shake();

void start_shake(ecs::entity e, float time = 2.0f, float strength = 16.0f);

}


