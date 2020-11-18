#include "hitTest.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

using ecs::entity;

entity hitTest2D(entity e, float2 parentPosition) {
    const auto& node = e.get<Node>();
    if ((node.flags & Node::VisibleAndTouchable) != Node::VisibleAndTouchable) {
        return nullptr;
    }

    float2 local = parentPosition;
    const auto* transform = e.tryGet<Transform2D>();
    if (transform && !transform->matrix.transform_inverse(local, local)) {
        // fail to make transform, discard
        return nullptr;
    }
    auto* bounds = e.tryGet<Bounds2D>();
    if (bounds) {
        if (!bounds->rect.contains(local)) {
            return nullptr;
        } else if (bounds->hitArea) {
            return e;
        }
    }

    auto it = node.child_last;
    while (it) {
        auto hit = hitTest2D(it, local);
        if (hit) {
            return hit;
        }
        it = it.get<Node>().sibling_prev;
    }

    const auto* display = e.tryGet<Display2D>();
    if (display && display->hitTest(local)) {
        return e;
    }

    return nullptr;
}

}
