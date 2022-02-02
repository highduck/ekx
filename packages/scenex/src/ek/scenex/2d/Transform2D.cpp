#include "Transform2D.hpp"

#include <ek/ds/FixedArray.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

vec2_t Transform2D::transformUp(ecs::EntityApi it, ecs::EntityApi top, vec2_t pos) {
    vec2_t result = pos;
    while (it && it != top) {
        const auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            result = vec2_transform(result, transform->matrix);
        }
        it = it.get<Node>().parent;
    }
    return result;
}

vec2_t Transform2D::transformDown(ecs::EntityApi top, ecs::EntityApi it, vec2_t pos) {
    vec2_t result = pos;
    while (it != top && it != nullptr) {
        const auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            vec2_transform_inverse(result, transform->matrix, &result);
        }
        it = it.get<Node>().parent;
    }
    return result;
}

vec2_t Transform2D::localToLocal(ecs::EntityApi src, ecs::EntityApi dst, vec2_t pos) {
    vec2_t result = pos;
    const auto lca = Node::findLowerCommonAncestor(src, dst);
    if (lca) {
        result = Transform2D::transformUp(src, lca, result);
        result = Transform2D::transformDown(lca, dst, result);
    }
    return result;
}

vec2_t Transform2D::localToGlobal(ecs::EntityApi local, vec2_t localPos) {
    vec2_t pos = localPos;
    auto it = local;
    while (it) {
        auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            pos = vec2_transform(pos, transform->matrix);
        }
        it = it.get<Node>().parent;
    }
    return pos;
}

vec2_t Transform2D::globalToLocal(ecs::EntityApi local, vec2_t globalPos) {
    vec2_t pos = globalPos;
    auto it = local;
    while (it) {
        auto* transform = it.tryGet<Transform2D>();
        if (transform) {
            vec2_transform_inverse(pos, transform->matrix, &pos);
        }

        it = it.get<Node>().parent;
    }
    return pos;
}


/** transformations after invalidation (already have world matrix) **/
void Transform2D::fastLocalToLocal(ecs::EntityApi src, ecs::EntityApi dst, vec2_t pos, vec2_t* out) {
    pos = vec2_transform(pos, src.get<WorldTransform2D>().matrix);
    vec2_transform_inverse(pos, dst.get<WorldTransform2D>().matrix, out);
}

/** Invalidate Transform2D **/

// idea to keep index to level start and process entities from that index to next level
//void traverseNodesBreathFirst(ecs::World* w, ecs::EntityApi root, std::vector<entity_t>& out) {
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
//    static std::vector<entity_t> vec2{};
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

void updateWorldTransformAll2(ecs::EntityApi root) {
    FixedArray<entity_t, ECX_ENTITIES_MAX_COUNT> out;
    out.push_back(root.index);

    uint32_t begin = 0;
    uint32_t end = out.size();
//    const auto* nodes = ecx.getStorage<Node>();
//    const auto* localTransforms = ecx.getStorage<Transform2D>();
//    const auto* worldTransforms = ecx.getStorage<WorldTransform2D>();

    /// copy transforms for all roots
//    worldTransforms->get(root.index).matrix = localTransforms->get(root.index).matrix;
//    worldTransforms->get(root.index).color = localTransforms->get(root.index).color;
    ecs::C<WorldTransform2D>::get_by_entity(root.index)->matrix = ecs::C<Transform2D>::get_by_entity(
            root.index)->matrix;
    ecs::C<WorldTransform2D>::get_by_entity(root.index)->color = ecs::C<Transform2D>::get_by_entity(root.index)->color;
    ///

    while (begin < end) {
        for (uint32_t i = begin; i < end; ++i) {
            const auto parent = out[i];
            const auto* tp = ecs::C<WorldTransform2D>::get_by_entity(parent);
            const auto* node = ecs::C<Node>::get_by_entity(parent);

            auto it = node->child_first.index;
            while (it) {
                auto* tw = ecs::C<WorldTransform2D>::get_by_entity(it);
                auto* tl = ecs::C<Transform2D>::get_by_entity(it);
                tw->matrix = mat3x2_mul(tp->matrix, tl->matrix);
                color2_mul(&tw->color, tp->color, tl->color);

                out.push_back(it);
//                auto* node_data = ecs::C<Node>::get_by_entity(it);
//                auto it_next = node_data->sibling_next.index;
//                if(it_next == it) {
//                    log_error("error invalid children in %s", hsp_get(node_data.tag));
//                    it_next = 0;
//                }
//                it = it_next;
                it = ecs::C<Node>::get_by_entity(it)->sibling_next.index;
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