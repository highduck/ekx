#include "hitTest.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

entity_t hitTest2D(entity_t e, const Node& node, vec2_t parentPosition) {
    if (node.flags & (NODE_UNTOUCHABLE | NODE_HIDDEN)) {
        return NULL_ENTITY;
    }

    vec2_t local = parentPosition;
    const auto* transform = ecs::try_get<Transform2D>(e);
    if (LIKELY(transform)) {
        const bool ok = vec2_transform_inverse(local, transform->matrix, &local);
        // TODO: unlikely
        if (UNLIKELY(!ok)) {
            // invalid transform, break
            return NULL_ENTITY;
        }
    }

    auto* bounds = ecs::try_get<Bounds2D>(e);
    if (bounds && !rect_contains(bounds->rect, local)) {
        return NULL_ENTITY;
    }

    entity_t it = node.child_last;
    while (it.id) {
        const auto& child_node = ecs::get<Node>(it);
        auto hit = hitTest2D(it, child_node, local);
        if (hit.id) {
            return hit;
        }
        it = child_node.sibling_prev;
    }

    const auto* display = ecs::try_get<Display2D>(e);
    if (display && display->hit_test && display->hit_test(e, local)) {
        return e;
    }

    if (bounds && (bounds->flags & BOUNDS_2D_HIT_AREA) && rect_contains(bounds->rect, local)) {
        return e;
    }

    return NULL_ENTITY;
}

entity_t hitTest2D(entity_t e, vec2_t parentPosition) {
    return hitTest2D(e, ecs::get<Node>(e), parentPosition);
}

}
