#pragma once

#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/components/event_handler.hpp>
#include <ek/scenex/data/sg_data.hpp>
#include <ek/scenex/components/script.hpp>
#include <ek/util/locator.hpp>

namespace ek {

static uint8_t camera_layers = 0xFFu;

void drawScene2D(ecs::entity root);
void drawSceneGizmos(ecs::entity root);

void draw_node(ecs::entity e);

ecs::entity hitTest(ecs::entity entity, float2 parentSpacePosition);

}