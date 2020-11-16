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

void updateScripts();

static uint8_t camera_layers = 0xFFu;

void drawScene2D(ecs::entity root);
void drawSceneGizmos(ecs::entity root);

void draw_node(ecs::entity e);

ecs::entity hit_test(ecs::entity entity, float2 parentSpacePosition);

void invalidateTransform2DRoot(ecs::entity root);

}