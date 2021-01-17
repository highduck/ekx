#include "Transform2D.hpp"

#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/base/Node.hpp>
#include "../../tracy/Tracy.hpp"

namespace ek {

void begin_transform(const Transform2D& transform) {
    const_cast<Transform2D&>(transform).updateLocalMatrix();
    draw2d::state.save_transform()
            .concat(transform.color.scale, transform.color.offset)
            .concat(transform.matrix);
}

void end_transform() {
    draw2d::state.restore_transform();
}

void Transform2D::updateLocalMatrix() {
    const auto x = position.x + origin.x;
    const auto y = position.y + origin.y;
    const auto xx = -origin.x - pivot.x;
    const auto yy = -origin.y - pivot.y;

    const auto ra = cosf(skew.y) * scale.x;
    const auto rb = sinf(skew.y) * scale.x;
    const auto rc = -sinf(skew.x) * scale.y;
    const auto rd = cosf(skew.x) * scale.y;

    matrix.a = ra;
    matrix.b = rb;
    matrix.c = rc;
    matrix.d = rd;
    matrix.tx = x + ra * xx + rc * yy;
    matrix.ty = y + rd * yy + rb * xx;
}

ecs::entity Transform2D::updateLocalMatrixSubTree(ecs::entity src, ecs::entity dst) {
    auto lca = Node::findLowerCommonAncestor(src, dst);
    if (lca) {
        auto it = src;
        while (it && it != lca) {
            auto* transform = it.tryGet<Transform2D>();
            if (transform) {
                transform->updateLocalMatrix();
            }
            it = it.get<Node>().parent;
        }

        it = dst;
        while (it && it != lca) {
            auto* transform = it.tryGet<Transform2D>();
            if (transform) {
                transform->updateLocalMatrix();
            }
            it = it.get<Node>().parent;
        }

        auto* transform = lca.tryGet<Transform2D>();
        if (transform) {
            transform->updateLocalMatrix();
        }
    }
    return lca;
}

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
        if (transform && !transform->matrix.transform_inverse(result, result)) {
            break;
        }
        it = it.get<Node>().parent;
    }
    return result;
}

float2 Transform2D::localToLocal(ecs::entity src, ecs::entity dst, float2 pos) {
    float2 result = pos;
    const auto lca = updateLocalMatrixSubTree(src, dst);
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
            transform->updateLocalMatrix();
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
            transform->updateLocalMatrix();
            if (!transform->matrix.transform_inverse(pos, pos)) {
                break;
            }
        }

        it = it.get<Node>().parent;
    }
    return pos;
}


/** transformations after invalidation (already have world matrix) **/
bool Transform2D::fastLocalToLocal(ecs::entity src, ecs::entity dst, float2 pos, float2& out) {
    pos = src.get<WorldTransform2D>().matrix.transform(pos);
    return dst.get<WorldTransform2D>().matrix.transform_inverse(pos, out);
}

/** Invalidate Transform2D **/

void updateAllLocalMatrices() {
    ZoneScoped;
    Transform2D* localTransforms = ecs::world_tryGetComponents<Transform2D>(&ecs::the_world)->get_ptr_by_handle(0);
    const uint32_t count = (uint32_t) ecs::world_tryGetComponents<Transform2D>(&ecs::the_world)->count;
    for (uint32_t i = 1; i < count; ++i) {
        localTransforms[i].updateLocalMatrix();
    }
}

// idea to keep index to level start and process entities from that index to next level
void traverseNodesBreathFirst(ecs::world* w, ecs::entity root, std::vector<ecs::Entity>& out) {
    ZoneScoped;
    out.push_back(root.index);
    uint32_t begin = 0;
    uint32_t end = 1;

    while (begin < end) {
        for (uint32_t i = begin; i < end; ++i) {
            const auto& node = ecs::entity_get<Node>(w, out[i]);
            auto it = node.child_first.index;
            while (it) {
                const auto& childNode = ecs::entity_get<Node>(w, it);
//                if (childNode.visible()) {
                out.push_back(it);
//                }
                it = childNode.sibling_next.index;
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
    for (auto entity : vec2) {
        auto& tw = ecs::entity_get<WorldTransform2D>(w, entity);
        const auto parent = ecs::entity_get<Node>(w, entity).parent.index;
        const auto& tp = ecs::entity_get<WorldTransform2D>(w, parent);
        auto& tl = ecs::entity_get<Transform2D>(w, entity);
        tl.updateLocalMatrix();
        tw.matrix = tp.matrix * tl.matrix;
        tw.color = tp.color * tl.color;
    }
}

void updateWorldTransform(ecs::entity e, const WorldTransform2D* transform) {
//    auto* localTransform = e.tryGet<Transform2D>();

    auto* worldTransform = e.tryGet<WorldTransform2D>();
    if (worldTransform) {
        const auto& localTransform = e.get_or_default<Transform2D>();
        worldTransform->matrix = transform->matrix * localTransform.matrix;
        worldTransform->color = transform->color * localTransform.color;
        transform = worldTransform;
    }

    auto it = e.get<Node>().child_first;
    while (it) {
        const auto& child = it.get<Node>();
        if (child.visible()) {
            updateWorldTransform(it, transform);
        }
        it = child.sibling_next;
    }
}
//
//void updateWorldTransform2D(ecs::entity root) {
//    ZoneScoped;
//    Transform2D* localTransforms = ecs::world_tryGetComponents<Transform2D>(&ecs::the_world)->get_ptr_by_handle(0);
//    const uint32_t count = (uint32_t) ecs::world_tryGetComponents<Transform2D>(&ecs::the_world)->count;
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