#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math.h>

namespace ek {

struct Node;

ecs::EntityIndex hitTest2D(const ecs::World& w, ecs::EntityIndex e, vec2_t parentSpacePosition);

}