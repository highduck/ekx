#include "Transform2D.hpp"

#include <ek/ds/SmallArray.hpp>
#include <ek/scenex/base/Node.hpp>
#include <Tracy.hpp>

namespace ek {

float2 Transform2D::transformUp(ecs::entity it, ecs::entity top, float2 pos) {
    float2 result = pos;
    while (it && it != top) {
        const auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            result = transform->matrix.transform(result);
        }
        it = it.get<Node>().parent;
    }
    return result;
}

float2 Transform2D::transformDown(ecs::entity top, ecs::entity it, float2 pos) {
    float2 result = pos;
    while (it != top && it != nullptr) {
        const auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            transform->matrix.transform_inverse(result, result);
        }
        it = it.get<Node>().parent;
    }
    return result;
}

float2 Transform2D::localToLocal(ecs::entity src, ecs::entity dst, float2 pos) {
    float2 result = pos;
    const auto lca = Node::findLowerCommonAncestor(src, dst);
    if (lca) {
        result = Transform2D::transformUp(src, lca, result);
        result = Transform2D::transformDown(lca, dst, result);
    }
    return result;
}

float2 Transform2D::localToGlobal(ecs::entity local, float2 localPos) {
    float2 pos = localPos;
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

float2 Transform2D::globalToLocal(ecs::entity local, float2 globalPos) {
    float2 pos = globalPos;
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
void Transform2D::fastLocalToLocal(ecs::entity src, ecs::entity dst, float2 pos, float2& out) {
    pos = src.get<WorldTransform2D>().matrix.transform(pos);
    dst.get<WorldTransform2D>().matrix.transform_inverse(pos, out);
}

/** Invalidate Transform2D **/

// idea to keep index to level start and process entities from that index to next level
void traverseNodesBreathFirst(ecs::world* w, ecs::entity root, std::vector<ecs::Entity>& out) {
    ZoneScoped;
    out.push_back(root.index);
    uint32_t begin = 0;
    uint32_t end = 1;
    const auto* nodes = w->getStorage<Node>();

    while (begin < end) {
        for (uint32_t i = begin; i < end; ++i) {
            const auto& node = nodes->get(out[i]);
            auto it = node.child_first.index;
            while (it) {
                out.push_back(it);
                it = nodes->get(it).sibling_next.index;
            }
        }
        begin = end;
        end = static_cast<uint32_t>(out.size());
    }
}

void updateWorldTransformAll(ecs::world* w, ecs::entity root) {
    ZoneScoped;
    static std::vector<ecs::Entity> vec2{};
    vec2.clear();
    traverseNodesBreathFirst(w, root, vec2);

    {
        const auto* localTransforms = w->getStorage<Transform2D>();
        const auto* worldTransforms = w->getStorage<WorldTransform2D>();
        const auto* nodes = w->getStorage<Node>();
        for (auto entity : vec2) {
            auto& tw = worldTransforms->get(entity);
            const auto& tp = worldTransforms->get( /* parent */ nodes->get(entity).parent.index);
            auto& tl = localTransforms->get(entity);
            matrix_2d::multiply(tp.matrix, tl.matrix, tw.matrix);
            ColorMod32::multiply(tp.color, tl.color, tw.color);
        }
    }
}

void updateWorldTransformAll2(ecs::world* w, ecs::entity root) {
    ZoneScoped;

    SmallArray<ecs::Entity, ecs::ENTITIES_MAX_COUNT> out;
    out.push(root.index);

    uint32_t begin = 0;
    uint32_t end = out.size;
    const auto* nodes = w->getStorage<Node>();
    const auto* localTransforms = w->getStorage<Transform2D>();
    const auto* worldTransforms = w->getStorage<WorldTransform2D>();

    /// copy transforms for all roots
    worldTransforms->get(root.index).matrix = localTransforms->get(root.index).matrix;
    worldTransforms->get(root.index).color = localTransforms->get(root.index).color;
    ///

    while (begin < end) {
        for (uint32_t i = begin; i < end; ++i) {
            const auto parent = out.data[i];
            const auto& tp = worldTransforms->get(parent);
            const auto& node = nodes->get(parent);

            auto it = node.child_first.index;
            while (it) {
                auto& tw = worldTransforms->get(it);
                auto& tl = localTransforms->get(it);
                matrix_2d::multiply(tp.matrix, tl.matrix, tw.matrix);
                ColorMod32::multiply(tp.color, tl.color, tw.color);

                out.push(it);
                it = nodes->get(it).sibling_next.index;
            }
        }
        begin = end;
        end = out.size;
    }
}
//
//void updateWorldTransform(ecs::entity e, const WorldTransform2D* transform) {
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
//void updateWorldTransform2D(ecs::entity root) {
//    ZoneScoped;
//    Transform2D* localTransforms = ecs::world_tryGetComponents<Transform2D>(ecs::the_world)->get_ptr_by_handle(0);
//    const uint32_t count = (uint32_t) ecs::world_tryGetComponents<Transform2D>(ecs::the_world)->count;
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