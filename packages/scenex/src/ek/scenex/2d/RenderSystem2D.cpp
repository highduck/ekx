#include "RenderSystem2D.hpp"
#include "Display2D.hpp"
#include "Transform2D.hpp"
#include "Camera2D.hpp"

#include <ek/canvas.h>
#include <ek/scenex/base/Node.hpp>

namespace ek {

int RenderSystem2D::currentLayerMask = 0xFF;

void RenderSystem2D::draw(const ecs::World& w, ecs::EntityIndex e, const WorldTransform2D* worldTransform) {
    EK_ASSERT(w.isValid(e));

    auto* bounds = w.tryGet<Bounds2D>(e);
    if (bounds) {
        const auto* camera = Camera2D::getCurrentRenderingCamera();
        auto rc = bounds->getScreenRect(camera->worldToScreenMatrix, worldTransform->matrix);
        if (Camera2D::getCurrentRenderingCamera()->occlusionEnabled) {
            if (!rect_overlaps(rc, camera->screenRect) || !rect_overlaps(rc, canvas.scissors[0])) {
                // discard
                return;
            }
        }
        if (bounds->scissors) {
            canvas_push_scissors(rc);
        }
    }

    bool programChanged = false;
    auto* display = w.tryGet<Display2D>(e);
    if (display) {
        if (display->program) {
            programChanged = true;
            canvas_push_program(REF_RESOLVE(res_shader, display->program));
        }
        if (display->drawable) {
            canvas.matrix[0] = worldTransform->matrix;
            canvas.color[0] = worldTransform->color;
            display->drawable->draw();
        }
    }

    auto it = w.get<Node>(e).child_first;
    while (it) {
        const auto& child = it.get<Node>();
        if (child.visible() && (child.layersMask() & currentLayerMask) != 0) {
            const auto* childWorldTransform = it.tryGet<WorldTransform2D>();
            if (childWorldTransform) {
                worldTransform = childWorldTransform;
            }
            if (worldTransform->color.scale.a > 0) {
                draw(w, it.index, worldTransform);
            }
        }
        it = child.sibling_next;
    }

    if (bounds && bounds->scissors) {
        canvas_pop_scissors();
    }
    if (programChanged) {
        canvas_restore_program();
    }
}


void RenderSystem2D::drawStack(const ecs::World& w, ecs::EntityIndex e) {
    EK_ASSERT(w.isValid(e));

    auto* bounds = w.tryGet<Bounds2D>(e);
    if (bounds) {
        const auto* camera = Camera2D::getCurrentRenderingCamera();
        auto rc = bounds->getScreenRect(camera->worldToScreenMatrix, canvas.matrix[0]);
        if (Camera2D::getCurrentRenderingCamera()->occlusionEnabled) {
            if (!rect_overlaps(rc, camera->screenRect) || !rect_overlaps(rc, canvas.scissors[0])) {
                // discard
                return;
            }
        }
        if (bounds->scissors) {
            canvas_push_scissors(rc);
            //draw2d::push_scissors(scissors->world_rect(transform->worldMatrix));
        }
    }

    bool programChanged = false;
    auto* display = w.tryGet<Display2D>(e);
    if (display) {
        if (display->program) {
            programChanged = true;
            canvas_push_program(REF_RESOLVE(res_shader, display->program));
        }
        if (display->drawable) {
            display->drawable->draw();
        }
    }

    auto it = w.get<Node>(e).child_first;
    while (it) {
        const auto& child = it.get<Node>();
        if (child.visible() && (child.layersMask() & currentLayerMask) != 0) {
            const auto* childTransform = it.tryGet<Transform2D>();
            if (childTransform) {
                canvas_save_transform();
                canvas_concat_matrix(childTransform->matrix);
                canvas_concat_color(childTransform->color);
            }
            if (canvas.color[0].scale.a != 0) {
                drawStack(w, it.index);
            }
            if (childTransform) {
                canvas_restore_transform();
            }
        }
        it = child.sibling_next;
    }

    if (bounds && bounds->scissors) {
        canvas_pop_scissors();
    }
    if (programChanged) {
        canvas_restore_program();
    }
}
}