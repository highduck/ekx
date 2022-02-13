#include "Transform2D.hpp"

#include <ek/ds/FixedArray.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

vec2_t transform_up(entity_t it, entity_t top, vec2_t pos) {
    vec2_t result = pos;
    while (it.id && it.id != top.id) {
        const Transform2D* transform = ecs::try_get<Transform2D>(it);
        if (transform) {
            result = vec2_transform(result, transform->matrix);
        }
        it = get_parent(it);
    }
    return result;
}

vec2_t transform_down(entity_t top, entity_t it, vec2_t pos) {
    vec2_t result = pos;
    while (it.id && it.id != top.id) {
        const Transform2D* transform = ecs::try_get<Transform2D>(it);
        if (transform) {
            vec2_transform_inverse(result, transform->matrix, &result);
        }
        it = get_parent(it);
    }
    return result;
}

vec2_t local_to_local(entity_t src, entity_t dst, vec2_t pos) {
    vec2_t result = pos;
    const entity_t lca = find_lower_common_ancestor(src, dst);
    if (lca.id) {
        result = transform_up(src, lca, result);
        result = transform_down(lca, dst, result);
    }
    return result;
}

vec2_t local_to_global(entity_t local, vec2_t localPos) {
    vec2_t pos = localPos;
    entity_t it = local;
    while (it.id) {
        Transform2D* transform = ecs::try_get<Transform2D>(it);
        if (transform) {
            pos = vec2_transform(pos, transform->matrix);
        }
        it = get_parent(it);
    }
    return pos;
}

vec2_t global_to_local(entity_t local, vec2_t globalPos) {
    vec2_t pos = globalPos;
    entity_t it = local;
    while (it.id) {
        Transform2D* transform = ecs::try_get<Transform2D>(it);
        if (transform) {
            vec2_transform_inverse(pos, transform->matrix, &pos);
        }
        it = get_parent(it);
    }
    return pos;
}


/** transformations after invalidation (already have world matrix) **/
void fast_local_to_local(entity_t src, entity_t dst, vec2_t pos, vec2_t* out) {
    pos = vec2_transform(pos, ecs::get<WorldTransform2D>(src).matrix);
    vec2_transform_inverse(pos, ecs::get<WorldTransform2D>(dst).matrix, out);
}

/** Invalidate Transform2D **/

void update_world_transform_2d(entity_t root) {
    FixedArray<entity_t, ECX_ENTITIES_MAX_COUNT> out;
    out.push_back(root);

    uint32_t begin = 0;
    uint32_t end = out.size();
    ecx_component_type* w_type = ecs::type<WorldTransform2D>();
    ecx_component_type* l_type = ecs::type<Transform2D>();
    ecx_component_type* node_type = ecs::type<Node>();
    WorldTransform2D* w_data = (WorldTransform2D*)w_type->data[0];
    Transform2D* l_data = (Transform2D*)l_type->data[0];
    Node* node_data = (Node*)node_type->data[0];
    component_handle_t w_handle = get_component_handle(w_type, root);
    component_handle_t l_handle = get_component_handle(l_type, root);
    /// copy transforms for all roots
    w_data[w_handle].matrix = l_data[l_handle].matrix;
    w_data[w_handle].color = l_data[l_handle].color;
    ///

    while (begin < end) {
        for (uint32_t i = begin; i < end; ++i) {
            const entity_t parent = out[i];
            const WorldTransform2D tp = w_data[get_component_handle(w_type, parent)];

            entity_t it = node_data[get_component_handle(node_type, parent)].child_first;
            while (it.id) {
                WorldTransform2D* tw = &w_data[get_component_handle(w_type, it)];
                const Transform2D tl = l_data[get_component_handle(l_type, it)];
                tw->matrix = mat3x2_mul(tp.matrix, tl.matrix);
                color2_mul(&tw->color, tp.color, tl.color);

                out.push_back(it);
//                auto* node_data = ecs::C<Node>::get_by_entity(it);
//                auto it_next = node_data->sibling_next.index;
//                if(it_next == it) {
//                    log_error("error invalid children in %s", hsp_get(node_data.tag));
//                    it_next = 0;
//                }
//                it = it_next;
                it =  node_data[get_component_handle(node_type, it)].sibling_next;
                //it = ecs::C<Node>::get_by_entity(it)->sibling_next.index;
            }
        }
        begin = end;
        end = out.size();
    }
}

}