#include "hitTest.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

ecs::EntityIndex hitTest2D(const ecs::World& w, ecs::EntityIndex e, const Node& node, vec2_t parentPosition) {
    if ((node.flags & Node::VisibleAndTouchable) != Node::VisibleAndTouchable) {
        return 0;
    }

    vec2_t local = parentPosition;
    const auto* transform = w.tryGet<Transform2D>(e);
    if (transform) {
        const bool ok = vec2_transform_inverse(local, transform->matrix, &local);
        // TODO: unlikely
        if(!ok) {
            // invalid transform, break
            return 0;
        }
    }

    auto* bounds = w.tryGet<Bounds2D>(e);
    if (bounds && !rect_contains(bounds->rect, local)) {
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

    if (bounds && bounds->hitArea && rect_contains(bounds->rect, local)) {
        return e;
    }

    return 0;
}

ecs::EntityIndex hitTest2D(const ecs::World& w, ecs::EntityIndex e, vec2_t parentPosition) {
    return hitTest2D(w, e, w.get<Node>(e), parentPosition);
}

}
