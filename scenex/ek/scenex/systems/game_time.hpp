#pragma once


#include <ecxx/ecxx.hpp>

namespace ek {

struct game_time_t {
    float dt_raw = 0.0f;
    float dt = 0.0f;
    float scale = 1.0f;
};

void update_time(ecs::entity root, float dt);
void update_node_time(ecs::entity e, float dt);
float get_delta_time(ecs::entity e);
void set_time_scale(ecs::entity e, float scale = 1.0f);
float get_raw_delta_time(ecs::entity e);

}


