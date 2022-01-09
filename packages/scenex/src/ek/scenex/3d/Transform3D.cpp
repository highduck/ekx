#include "Transform3D.hpp"
#include <ecxx/ecxx.hpp>
#include <ek/scenex/base/Node.hpp>

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
        tr.local = //mat4_translate_transform(tr.position);
                mat4_mul(
                        mat4_mul(
                                mat4_translate_transform(tr.position),
                                mat4_rotation_transform_quat(
                                        normalize_quat(quat_euler_angles(tr.rotation))
                                )
                        ),
                        mat4_scale_transform(tr.scale)
                );
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