#include "scene_system.hpp"
#include "game_time.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
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
    if (e.has<Transform2D>()) {
        auto& transform = e.get<Transform2D>();
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

    if (e.has<Display2D>() && e.get<Display2D>().hitTest(local)) {
        return e;
    }

    return nullptr;
}

void drawScene2DChildren(entity e);

void drawScene2D(entity root) {
    draw2d::state.save_transform();
    drawScene2DChildren(root);
    draw2d::state.restore_transform();
}

void draw_node(entity e) {
    assert(e.valid());

    if (process_node_filters(e)) {
        return;
    }

    //begin_transform(transform);

    auto* scissors = e.tryGet<scissors_2d>();
    if (scissors) {
        draw2d::state.push_scissors(scissors->world_rect(draw2d::state.matrix));
    }

    auto* display = e.tryGet<Display2D>();
    if (display && display->drawable) {
        display->drawable->draw();
    }

    auto* scripts = e.tryGet<script_holder>();
    if (scripts) {
        for (auto& script : scripts->list) {
            if (script) {
                script->draw();
            }
        }
    }

    drawScene2DChildren(e);

    if (scissors) {
        draw2d::state.pop_scissors();
    }
}

void drawScene2DChildren(entity e) {
    auto it = e.get<node_t>().child_first;
    while (it) {
        const auto& child = it.get<node_t>();
        if (child.visible() &&
            (child.layersMask() & camera_layers) != 0) {
            const auto& childTransform = it.get<Transform2D>();
            if (childTransform.worldColor.scale.a > 0) {
                draw2d::state.matrix = childTransform.worldMatrix;
                draw2d::state.color = childTransform.worldColor;
                draw_node(it);
            }
        }
        it = child.sibling_next;
    }
}

void updateScripts() {
    float dt = TimeLayer::Root->dt;
    ecs::rview<script_holder>()
            .each([dt](script_holder& scripts) {
                for (auto& script : scripts.list) {
                    if (script) {
                        script->update(dt);
                    }
                }
            });
}

/** gizmo drawing pass **/
void drawSceneGizmo(entity e) {
    assert(e.valid());
    auto* transform = e.tryGet<Transform2D>();
    if (transform) {
        auto* display = e.tryGet<Display2D>();
        if (display && display->drawable && display->drawBounds) {
            draw2d::state.matrix.set_identity();
            auto bounds = display->drawable->getBounds();
            auto v1 = transform->worldMatrix.transform(bounds.position);
            auto v2 = transform->worldMatrix.transform(bounds.right(), bounds.y);
            auto v3 = transform->worldMatrix.transform(bounds.right(), bounds.bottom());
            auto v4 = transform->worldMatrix.transform(bounds.x, bounds.bottom());
            draw2d::line(v1, v2, 0xFFFFFFFF_argb, 1);
            draw2d::line(v2, v3, 0xFFFFFFFF_argb, 1);
            draw2d::line(v3, v4, 0xFFFFFFFF_argb, 1);
            draw2d::line(v4, v1, 0xFFFFFFFF_argb, 1);
        }

        auto* scripts = e.tryGet<script_holder>();
        if (scripts) {
            draw2d::state.matrix = transform->worldMatrix;
            for (auto& script : scripts->list) {
                script->gui_gizmo();
            }
        }

        auto it = e.get<node_t>().child_first;
        while (it) {
            drawSceneGizmo(it);
            it = it.get<node_t>().sibling_next;
        }
    }
}

void drawSceneGizmos(entity root) {
    draw2d::state.save_transform();
    drawSceneGizmo(root);
    draw2d::state.restore_transform();
}

/** Invalidate Transform2D **/

void invalidateTransform2D(entity e, const Transform2D* transform) {
    auto* localTransform = e.tryGet<Transform2D>();
    if (localTransform) {
        localTransform->updateLocalMatrix();
        localTransform->worldMatrix = transform->worldMatrix * localTransform->matrix;
        localTransform->worldColor = transform->worldColor * localTransform->color;
        transform = localTransform;
    }
    auto it = e.get<node_t>().child_first;
    while (it) {
        const auto& child = it.get<node_t>();
        if (child.visible()) {
            invalidateTransform2D(it, transform);
        }
        it = child.sibling_next;
    }
}

void invalidateTransform2DRoot(entity root) {
    auto* transform = root.tryGet<Transform2D>();
    assert(transform != nullptr);
    transform->updateLocalMatrix();
    transform->worldMatrix = transform->matrix;
    transform->worldColor = transform->color;

    auto it = root.get<node_t>().child_first;
    while (it) {
        const auto& child = it.get<node_t>();
        if (child.visible()) {
            invalidateTransform2D(it, transform);
        }
        it = child.sibling_next;
    }
}

}
