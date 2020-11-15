#include "scene_system.hpp"

#include <ek/scenex/particles/particle_layer.hpp>
#include <ek/scenex/particles/particle_system.hpp>
#include <ek/scenex/components/transform_2d.hpp>
#include <ek/scenex/components/display_2d.hpp>
#include <ek/scenex/components/node_filters.hpp>
#include <ek/scenex/data/sg_factory.hpp>

namespace ek {

using ecs::entity;

entity hit_test(entity e, float2 parentPosition) {
    const auto& node = e.get<node_t>();
    if ((node.flags & NodeFlags_VisibleAndTouchable) != NodeFlags_VisibleAndTouchable) {
        return nullptr;
    }

    float2 local = parentPosition;
    if (e.has<transform_2d>()) {
        auto& transform = e.get<transform_2d>();
        if (!transform.matrix.transform_inverse(local, local)) {
            return nullptr;
        }
        if (e.has<scissors_2d>() && !e.get<scissors_2d>().rect.contains(local.x, local.y)) {
            return nullptr;
        }

        if (e.has<hit_area_2d>()) {
            return e.get<hit_area_2d>().rect.contains(local.x, local.y) ? e : nullptr;
        }

        //if(e.has<Bounds2D>()) {
        // TODO: check if not drawable out of scope!
        //}
    }

    auto it = node.child_last;
    while (it) {
        auto hit = hit_test(it, local);
        if (hit) {
            return hit;
        }
        it = it.get<node_t>().sibling_prev;
    }

    if (e.has<display_2d>() && e.get<display_2d>().hitTest(local)) {
        return e;
    }

    return nullptr;
}

void draw_node(entity e) {
    assert(e.valid());

    if (process_node_filters(e)) {
        return;
    }

    const auto& node = e.get_or_default<node_t>();
    auto& transform = e.get_or_default<transform_2d>();

    if (!node.visible() || transform.color_multiplier.a <= 0 || (node.layersMask() & camera_layers) == 0) {
        return;
    }

    begin_transform(transform);

    bool scissors = ecs::has<scissors_2d>(e);
    if (scissors) {
        draw2d::state.push_scissors(
                ecs::get<scissors_2d>(e).world_rect(
                        draw2d::state.matrix
                )
        );
    }
//
//    events.emit({PreDraw, this, nullptr});
    if (e.has<display_2d>()) {
        auto& display = e.get<display_2d>();
        if (display.drawable) {
            display.drawable->draw();

#ifndef NDEBUG
            if (display.drawBounds) {
                draw2d::strokeRect(expand(display.drawable->get_bounds(), 1.0f), 0xFFFFFFFF_argb, 1);
            }
#endif
        }
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
        draw2d::state.pop_scissors();
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
