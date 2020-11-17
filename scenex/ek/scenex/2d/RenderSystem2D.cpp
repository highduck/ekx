#include "RenderSystem2D.hpp"
#include "Display2D.hpp"
#include "Transform2D.hpp"

#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/components/Node.hpp>
#include <ek/scenex/components/script.hpp>

namespace ek {

int RenderSystem2D::currentLayerMask = 0xFF;

void RenderSystem2D::draw(ecs::entity e, const Transform2D* transform) {
    assert(e.valid());

    // todo:
//    if (process_node_filters(e)) {
//        return;
//    }

    auto* scissors = e.tryGet<scissors_2d>();
    if (scissors) {
        draw2d::state.push_scissors(scissors->world_rect(transform->worldMatrix));
    }

    auto* display = e.tryGet<Display2D>();
    if (display && display->drawable) {
        draw2d::state.matrix = transform->worldMatrix;
        draw2d::state.color = transform->worldColor;
        display->drawable->draw();
    }

    auto* scripts = e.tryGet<script_holder>();
    if (scripts) {
        draw2d::state.matrix = transform->worldMatrix;
        draw2d::state.color = transform->worldColor;
        for (auto& script : scripts->list) {
            if (script) {
                script->draw();
            }
        }
    }

    auto it = e.get<Node>().child_first;
    while (it) {
        const auto& child = it.get<Node>();
        if (child.visible() && (child.layersMask() & currentLayerMask) != 0) {
            const auto* childTransform = it.tryGet<Transform2D>();
            if (childTransform) {
                transform = childTransform;
            }
            if (transform->color.scale.a > 0) {
                draw(it, transform);
            }
        }
        it = child.sibling_next;
    }

    if (scissors) {
        draw2d::state.pop_scissors();
    }
}
}