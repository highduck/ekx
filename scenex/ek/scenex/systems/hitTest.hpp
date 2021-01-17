#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>

namespace ek {

struct Node;

ecs::entity hitTest2D(ecs::entity entity, float2 parentSpacePosition);

}