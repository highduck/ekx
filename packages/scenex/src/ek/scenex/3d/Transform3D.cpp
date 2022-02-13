#include "Transform3D.hpp"
#include <ecx/ecx.hpp>
#include <ek/scenex/base/Node.hpp>

//ecx_component_type COMP_Transform3D;
//
//Transform3D* get_transform3d(entity_t e) {
//    Transform3D* data = (Transform3D*)COMP_Transform3D.data[0];
//    component_handle_t handle = get_component_handle(&COMP_Transform3D, e);
//    return handle ? &data[handle] : NULL;
//}
//
//ek::Node* get_node(entity_t e) {
//    ek::Node* data = (ek::Node*)ecs::C<ek::Node>::type->data[0];
//    component_handle_t handle = get_component_handle(ecs::C<ek::Node>::type, e);
//    return handle ? &data[handle] : NULL;
//}

static void updateWorldMatrix3D(entity_t e, const mat4_t* parent) {
    Transform3D* tr = ecs::try_get<Transform3D>(e);
    if (tr) {
        tr->world = mat4_mul(tr->local, *parent);
        parent = &tr->world;
    }
    auto* node = ecs::try_get<ek::Node>(e);
    if (node) {
        auto it = node->child_first;
        while (it.id) {
            updateWorldMatrix3D(it, parent);
            it = ek::get_next_child(it);
        }
    }
}

void update_world_transform3d() {
    // TODO: correct hierarchy update
    for (auto e: ecs::view<Transform3D>()) {
        auto& tr = ecs::get<Transform3D>(e);
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
        auto* node = e.try_get<ek::Node>();
        if (!node || !node->parent.id) {
            updateWorldMatrix3D(e, &identity);
        }
    }
}
