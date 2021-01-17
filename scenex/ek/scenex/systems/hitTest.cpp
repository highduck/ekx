#include "hitTest.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

using ecs::entity;

entity hitTest2D(entity e, const Node& node, float2 parentPosition) {
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
        const auto& childNode = it.get<Node>();
        auto hit = hitTest2D(it, childNode, local);
        if (hit) {
            return hit;
        }
        it = childNode.sibling_prev;
    }

    const auto* display = e.tryGet<Display2D>();
    if (display && display->hitTest(local)) {
        return e;
    }

    return nullptr;
}

entity hitTest2D(entity e, float2 parentPosition) {
    return hitTest2D(e, e.get<Node>(), parentPosition);
}

}
