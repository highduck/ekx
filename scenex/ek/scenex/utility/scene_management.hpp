#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Transform2D.hpp>

namespace ek {

inline ecs::entity createNode2D(const char* name = nullptr) {
    auto e = ecs::create<Node, Transform2D>();
    if (name) {
        e.get<Node>().name = name;
    }
    return e;
}

}