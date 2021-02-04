#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>

namespace ek {

struct Node;

ecs::Entity hitTest2D(const ecs::world& w, ecs::Entity e, float2 parentSpacePosition);

}