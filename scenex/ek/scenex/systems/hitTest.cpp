#include "hitTest.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/components/Node.hpp>

namespace ek {

using ecs::entity;

entity hitTest2D(entity e, float2 parentPosition) {
    const auto& node = e.get<Node>();
    if ((node.flags & Node::VisibleAndTouchable) != Node::VisibleAndTouchable) {
        return nullptr;
    }

    float2 local = parentPosition;
    if (e.has<Transform2D>()) {
        auto& transform = e.get<Transform2D>();
        if (!transform.matrix.transform_inverse(local, local)) {
            return nullptr;
        }
        if (e.has<scissors_2d>() && !e.get<scissors_2d>().rect.contains(local.x, local.y)) {
            return nullptr;
        }

        if (e.has<hit_area_2d>()) {
            return e.get<hit_area_2d>().rect.contains(local.x, local.y) ? e : nullptr;
        }

        //if(e.has<Bounds2D>()) {
        // TODO: check if not drawable out of scope!
        //}
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
