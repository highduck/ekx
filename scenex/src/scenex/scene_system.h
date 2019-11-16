#pragma once

#include <draw2d/drawer.hpp>
#include <scenex/components/event_handler.h>
#include <scenex/data/sg_data.h>
#include <scenex/components/script_t.h>
#include <ek/locator.hpp>

namespace scenex {

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