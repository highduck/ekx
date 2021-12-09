#include "Transform2D.hpp"

#include <ek/ds/FixedArray.hpp>
#include <ek/scenex/base/Node.hpp>
#include <Tracy.hpp>

namespace ek {

Vec2f Transform2D::transformUp(ecs::EntityApi it, ecs::EntityApi top, Vec2f pos) {
    Vec2f result = pos;
    while (it && it != top) {
        const auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            result = transform->matrix.transform(result);
        }
        it = it.get<Node>().parent;
    }
    return result;
}

Vec2f Transform2D::transformDown(ecs::EntityApi top, ecs::EntityApi it, Vec2f pos) {
    Vec2f result = pos;
    while (it != top && it != nullptr) {
        const auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            transform->matrix.transform_inverse(result, result);
        }
        it = it.get<Node>().parent;
    }
    return result;
}

Vec2f Transform2D::localToLocal(ecs::EntityApi src, ecs::EntityApi dst, Vec2f pos) {
    Vec2f result = pos;
    const auto lca = Node::findLowerCommonAncestor(src, dst);
    if (lca) {
        result = Transform2D::transformUp(src, lca, result);
        result = Transform2D::transformDown(lca, dst, result);
    }
    return result;
}

Vec2f Transform2D::localToGlobal(ecs::EntityApi local, Vec2f localPos) {
    Vec2f pos = localPos;
    auto it = local;
    while (it) {
        auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            pos = transform->matrix.transform(pos);
        }
        it = it.get<Node>().parent;
    }
    return pos;
}

Vec2f Transform2D::globalToLocal(ecs::EntityApi local, Vec2f globalPos) {
    Vec2f pos = globalPos;
    auto it = local;
    while (it) {
        auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            transform->matrix.transform_inverse(pos, pos);
        }

        it = it.get<Node>().parent;
    }
    return pos;
}


/** transformations after invalidation (already have world matrix) **/
void Transform2D::fastLocalToLocal(ecs::EntityApi src, ecs::EntityApi dst, Vec2f pos, Vec2f& out) {
    pos = src.get<WorldTransform2D>().matrix.transform(pos);
    dst.get<WorldTransform2D>().matrix.transform_inverse(pos, out);
}

/** Invalidate Transform2D **/

// idea to keep index to level start and process entities from that index to next level
//void traverseNodesBreathFirst(ecs::World* w, ecs::EntityApi root, std::vector<ecs::EntityIndex>& out) {
//    ZoneScoped;
//    out.push_back(root.index);
//    uint32_t begin = 0;
//    uint32_t end = 1;
//    const auto* nodes = w->getStorage<Node>();
//
//    while (begin < end) {
//        for (uint32_t i = begin; i < end; ++i) {
//            const auto& node = nodes->get(out[i]);
//            auto it = node.child_first.index;
//            while (it) {
//                out.push_back(it);
//                it = nodes->get(it).sibling_next.index;
//            }
//        }
//        begin = end;
//        end = static_cast<uint32_t>(out.size());
//    }
//}
//
//void updateWorldTransformAll(ecs::World* w, ecs::EntityApi root) {
//    ZoneScoped;
//    static std::vector<ecs::EntityIndex> vec2{};
//    vec2.clear();
//    traverseNodesBreathFirst(w, root, vec2);
//
//    {
//        const auto* localTransforms = w->getStorage<Transform2D>();
//        const auto* worldTransforms = w->getStorage<WorldTransform2D>();
//        const auto* nodes = w->getStorage<Node>();
//        for (auto entity : vec2) {
//            auto& tw = worldTransforms->get(entity);
//            const auto& tp = worldTransforms->get( /* parent */ nodes->get(entity).parent.index);
//            auto& tl = localTransforms->get(entity);
//            Matrix3x2f::multiply(tp.matrix, tl.matrix, tw.matrix);
//            ColorMod32::multiply(tp.color, tl.color, tw.color);
//        }
//    }
//}

void updateWorldTransformAll2(ecs::World* w, ecs::EntityApi root) {
    ZoneScoped;

    FixedArray<ecs::EntityIndex, ecs::ENTITIES_MAX_COUNT> out;
    out.push_back(root.index);

    uint32_t begin = 0;
    uint32_t end = out.size();
    const auto* nodes = w->getStorage<Node>();
    const auto* localTransforms = w->getStorage<Transform2D>();
    const auto* worldTransforms = w->getStorage<WorldTransform2D>();

    /// copy transforms for all roots
    worldTransforms->get(root.index).matrix = localTransforms->get(root.index).matrix;
    worldTransforms->get(root.index).color = localTransforms->get(root.index).color;
    ///

    while (begin < end) {
        for (uint32_t i = begin; i < end; ++i) {
            const auto parent = out[i];
            const auto& tp = worldTransforms->get(parent);
            const auto& node = nodes->get(parent);

            auto it = node.child_first.index;
            while (it) {
                auto& tw = worldTransforms->get(it);
                auto& tl = localTransforms->get(it);
                Matrix3x2f::multiply(tp.matrix, tl.matrix, tw.matrix);
                ColorMod32::multiply(tp.color, tl.color, tw.color);

                out.push_back(it);
                it = nodes->get(it).sibling_next.index;
            }
        }
        begin = end;
        end = out.size();
    }
}
//
//void updateWorldTransform(ecs::EntityApi e, const WorldTransform2D* transform) {
//    auto* worldTransform = e.tryGet<WorldTransform2D>();
//    if (worldTransform) {
//        const auto& localTransform = e.get_or_default<Transform2D>();
//        worldTransform->matrix = transform->matrix * localTransform.matrix;
//        worldTransform->color = transform->color * localTransform.color;
//        transform = worldTransform;
//    }
//
//    auto it = e.get<Node>().child_first;
//    while (it) {
//        const auto& child = it.get<Node>();
//        if (child.visible()) {
//            updateWorldTransform(it, transform);
//        }
//        it = child.sibling_next;
//    }
//}
//
//void updateWorldTransform2D(ecs::EntityApi root) {
//    ZoneScoped;
//    Transform2D* localTransforms = ecs::World_tryGetComponents<Transform2D>(ecs::the_world)->get_ptr_by_handle(0);
//    const uint32_t count = (uint32_t) ecs::World_tryGetComponents<Transform2D>(ecs::the_world)->count;
//    for (uint32_t i = 1; i < count; ++i) {
//        localTransforms[i].updateLocalMatrix();
//    }
//
//    auto& transform = root.get<Transform2D>();
//    auto& worldTransform = root.get<WorldTransform2D>();
//    //transform.updateLocalMatrix();
//    worldTransform.matrix = transform.matrix;
//    worldTransform.color = transform.color;
//
//    auto it = root.get<Node>().child_first;
//    while (it) {
//        const auto& child = it.get<Node>();
//        if (child.visible()) {
//            updateWorldTransform(it, &worldTransform);
//        }
//        it = child.sibling_next;
//    }
//}
}