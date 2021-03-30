#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Transform2D.hpp>

namespace ek {

inline ecs::EntityApi createNode2D(const char* name = nullptr) {
    auto e = ecs::create<Node, Transform2D, WorldTransform2D>();
    if (name) {
        e.assign<NodeName>(name);
    }
    return e;
}

}