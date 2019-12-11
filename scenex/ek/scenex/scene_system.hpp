#pragma once

#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/components/event_handler.hpp>
#include <ek/scenex/data/sg_data.hpp>
#include <ek/scenex/components/script.hpp>
#include <ek/util/locator.hpp>

namespace ek {

template<typename Func>
inline void each_script(Func func) {
    ecs::view<script_holder>().each([&func](script_holder& holder) {
        for (auto& script : holder.list) {
            if (script) {
                func(*script);
            }
        }
    });
}


void update_nodes(ecs::entity entity, float dt);

static uint8_t camera_layers = 0xFFu;

void draw_node(ecs::entity e);

ecs::entity hit_test(ecs::entity entity, const float2& position);

}