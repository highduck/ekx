#include <ek/scenex/particles/particle_layer.hpp>
#include <ek/scenex/particles/particle_system.hpp>
#include <ek/scenex/components/transform_2d.hpp>
#include <ek/scenex/components/display_2d.hpp>
#include <ek/scenex/components/name.hpp>
#include <ek/scenex/components/node_filters.hpp>
#include "scene_system.hpp"
#include <ek/scenex/data/sg_factory.hpp>

namespace ek {

using ecs::entity;

entity hit_test(entity e, const float2& position) {
    const auto& config = ecs::get_or_default<node_state_t>(e);
    if (!config.visible || !config.touchable) {
        return ecs::null;
    }

    if (ecs::has<scissors_2d>(e) &&
        !ecs::get<scissors_2d>(e).rect.contains(position)) {
        return ecs::null;
    }

    if (ecs::has<hit_area_2d>(e)) {
        return ecs::get<hit_area_2d>(e).rect.contains(position) ? e : ecs::null;
    }

    auto it = ecs::get<node_t>(e).child_last;
    while (it) {
        float2 inverted;
        if (ecs::get<transform_2d>(it).matrix.transform_inverse(position, &inverted)) {
            auto hit = hit_test(it, inverted);
            if (hit) {
                return hit;
            }
        }
        it = ecs::get<node_t>(it).sibling_prev;
    }

    const auto& display = ecs::get_or_default<display_2d>(e);
    if (display.drawable && display.drawable->hit_test(position)) {
        return e;
    }

    return ecs::null;
}

void draw_node(entity e) {
    // debug
#ifndef NDEBUG
    const auto& name = ecs::get_or_default<name_t>(e);
#endif

    if (process_node_filters(e)) {
        return;
    }

    auto& drawer = resolve<drawer_t>();
    const auto& config = ecs::get_or_default<node_state_t>(e);

    auto& transform = ecs::get_or_default<transform_2d>(e);
    if (!config.visible || transform.colorMultiplier.a <= 0 || (config.layer_mask & camera_layers) == 0) {
        return;
    }

    begin_transform(transform);

    bool scissors = ecs::has<scissors_2d>(e);
    if (scissors) {
        drawer.begin_scissors(ecs::get<scissors_2d>(e).world_rect(drawer.matrix));
    }
//
//    events.emit({PreDraw, this, nullptr});

    const auto& display = ecs::get_or_default<display_2d>(e);
    if (display.drawable) {
        display.drawable->draw();
    }

    if (ecs::has<script_holder>(e)) {
        auto& holder = ecs::get<script_holder>(e);
        for (auto& script : holder.list) {
            if (script) {
                script->draw();
            }
        }
    }

    if (ecs::has<particle_layer_t>(e)) {
        draw_particle_layer(e);
    }
//    events.emit({OnDraw, this, nullptr});

    each_child(e, [](entity child) {
        draw_node(child);
    });

    if (scissors) {
        drawer.end_scissors();
    }

    if (ecs::has<script_holder>(e)) {
        auto& scripts = ecs::get<script_holder>(e).list;
        for (auto& script : scripts) {
            script->gui_gizmo();
        }
    }

    end_transform();
}

void update_nodes(entity e, float dt) {
    if (ecs::has<script_holder>(e)) {
        auto& holder = ecs::get<script_holder>(e);
        for (auto& script : holder.list) {
            if (script) {
                script->update(dt);
            }
        }
    }

    each_child(e, [dt](entity child) {
        update_nodes(child, dt);
    });
}

}
