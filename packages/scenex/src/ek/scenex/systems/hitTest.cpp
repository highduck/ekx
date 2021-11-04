#include "hitTest.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

ecs::EntityIndex hitTest2D(const ecs::World& w, ecs::EntityIndex e, const Node& node, float2 parentPosition) {
    if ((node.flags & Node::VisibleAndTouchable) != Node::VisibleAndTouchable) {
        return 0;
    }

    float2 local = parentPosition;
    const auto* transform = w.tryGet<Transform2D>(e);
    if (transform) {
        transform->matrix.transform_inverse(local, local);
    }

    auto* bounds = w.tryGet<Bounds2D>(e);
    if (bounds && !bounds->rect.contains(local)) {
        return 0;
    }

    auto it = node.child_last;
    while (it) {
        const auto& childNode = it.get<Node>();
        auto hit = hitTest2D(w, it.index, childNode, local);
        if (hit) {
            return hit;
        }
        it = childNode.sibling_prev;
    }

    const auto* display = w.tryGet<Display2D>(e);
    if (display && display->hitTest(local)) {
        return e;
    }

    if (bounds && bounds->hitArea && bounds->rect.contains(local)) {
        return e;
    }

    return 0;
}

ecs::EntityIndex hitTest2D(const ecs::World& w, ecs::EntityIndex e, float2 parentPosition) {
    return hitTest2D(w, e, w.get<Node>(e), parentPosition);
}

}
