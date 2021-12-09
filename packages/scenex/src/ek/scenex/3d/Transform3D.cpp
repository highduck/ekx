#include "Transform3D.hpp"
#include <ecxx/ecxx.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/math/Quaternion.hpp>
#include <ek/math/MatrixTransform.hpp>

namespace ek {

void updateWorldMatrix3D(ecs::EntityApi e, const Matrix4f* parent) {
    auto* tr = e.tryGet<Transform3D>();
    if (tr) {
        tr->world = tr->local * (*parent);
        parent = &tr->world;
    }
    auto* node = e.tryGet<Node>();
    if (node) {
        auto it = node->child_first;
        while (it) {
            updateWorldMatrix3D(it, parent);
            it = it.get<Node>().sibling_next;
        }
    }
}

void Transform3D::updateAll() {
    // TODO: correct hierarchy update
    for (auto e: ecs::view<Transform3D>()) {
        auto& tr = e.get<Transform3D>();
        tr.local = translate_transform(tr.position)
                   * rotation_transform(Quaternion<float>(tr.rotation))
                   * scale_transform(tr.scale);
    }
    static Matrix4f identity{};
    for (auto e: ecs::view<Transform3D>()) {
        auto* node = e.tryGet<Node>();
        if (!node || !node->parent) {
            updateWorldMatrix3D(e, &identity);
        }
    }
}

}