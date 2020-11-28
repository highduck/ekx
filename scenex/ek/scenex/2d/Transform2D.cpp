#include "Transform2D.hpp"

#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/base/Node.hpp>

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
            if (!it.get<Transform2D>().matrix.transform_inverse(pos, pos)) {
                break;
            }
        }

        it = it.get<Node>().parent;
    }
    return pos;
}


/** transformations after invalidation (already have world matrix) **/
bool Transform2D::fastLocalToLocal(ecs::entity src, ecs::entity dst, float2 pos, float2& out) {
    pos = src.get<Transform2D>().worldMatrix.transform(pos);
    return dst.get<Transform2D>().worldMatrix.transform_inverse(pos, out);
}

/** Invalidate Transform2D **/

void updateWorldTransform(ecs::entity e, const Transform2D* transform) {
    auto* localTransform = e.tryGet<Transform2D>();
    if (localTransform) {
        localTransform->updateLocalMatrix();
        localTransform->worldMatrix = transform->worldMatrix * localTransform->matrix;
        localTransform->worldColor = transform->worldColor * localTransform->color;
        transform = localTransform;
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

void updateWorldTransform2D(ecs::entity root) {
    auto& transform = root.get<Transform2D>();
    transform.updateLocalMatrix();
    transform.worldMatrix = transform.matrix;
    transform.worldColor = transform.color;

    auto it = root.get<Node>().child_first;
    while (it) {
        const auto& child = it.get<Node>();
        if (child.visible()) {
            updateWorldTransform(it, &transform);
        }
        it = child.sibling_next;
    }
}

}