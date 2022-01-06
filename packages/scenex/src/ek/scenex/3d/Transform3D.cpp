#include "Transform3D.hpp"
#include <ecxx/ecxx.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/math/Matrix.hpp>
#include <ek/math/Vec.hpp>
#include <ek/math/MatrixTransform.hpp>
#include <ek/math/Quaternion.hpp>

namespace ek {

void updateWorldMatrix3D(ecs::EntityApi e, const mat4_t* parent) {
    auto* tr = e.tryGet<Transform3D>();
    if (tr) {
        tr->world = mat4_mul(tr->local, *parent);
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
        tr.local = (translate_transform((Vec3f)tr.position)
                   * rotation_transform(Quaternion<float>((Vec3f)tr.rotation))
                   * scale_transform((Vec3f)tr.scale)).cdata;
    }
    static mat4_t identity = mat4_identity();
    for (auto e: ecs::view<Transform3D>()) {
        auto* node = e.tryGet<Node>();
        if (!node || !node->parent) {
            updateWorldMatrix3D(e, &identity);
        }
    }
}

}