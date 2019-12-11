#include "game_time.h"

#include <ek/scenex/components/node_t.h>

namespace ek {

using ecs::entity;

void update_node_time(entity e, float dt) {
    auto& time = ecs::get_or_create<game_time_t>(e);
    dt *= time.scale;
    time.dt = dt;
    auto it = ecs::get_or_default<node_t>(e).child_first;
    while (it) {
        update_node_time(it, dt);
        it = ecs::get_or_default<node_t>(it).sibling_next;
    }
}

void update_time(entity root, float dt) {
    for (auto e : ecs::view<game_time_t>()) {
        ecs::get<game_time_t>(e).dt_raw = dt;
        ecs::get<game_time_t>(e).dt = 0.0f;
    }
    update_node_time(root, dt);
}

float get_delta_time(entity e) {
    return ecs::get_or_default<game_time_t>(e).dt;
}

float get_raw_delta_time(entity e) {
    return ecs::get_or_default<game_time_t>(e).dt_raw;
}

void set_time_scale(ecs::entity e, float scale) {
    auto& t = ecs::get_or_create<game_time_t>(e);
    t.scale = scale;
}

}