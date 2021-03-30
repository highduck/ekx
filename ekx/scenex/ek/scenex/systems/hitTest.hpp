#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>

namespace ek {

struct Node;

ecs::EntityIndex hitTest2D(const ecs::World& w, ecs::EntityIndex e, float2 parentSpacePosition);

}