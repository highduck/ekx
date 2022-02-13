#include "RenderSystem2D.hpp"
#include "Display2D.hpp"
#include "Transform2D.hpp"
#include "Camera2D.hpp"

#include <ek/canvas.h>
#include <ek/scenex/base/Node.hpp>

namespace ek {

void RenderSystem2D::draw(entity_t e, const WorldTransform2D* worldTransform) {
    EK_ASSERT(is_entity(e));

    auto* bounds = ecs::try_get<Bounds2D>(e);
    if (bounds) {
        const auto* camera = Camera2D::getCurrentRenderingCamera();
        auto rc = bounds->getScreenRect(camera->worldToScreenMatrix, worldTransform->matrix);
        if (Camera2D::getCurrentRenderingCamera()->occlusionEnabled) {
            if (!rect_overlaps(rc, camera->screenRect) || !rect_overlaps(rc, canvas.scissors[0])) {
                // discard
                return;
            }
        }
        if (bounds->flags & BOUNDS_2D_SCISSORS) {
            canvas_push_scissors(rc);
        }
    }

    bool programChanged = false;
    auto* display = ecs::try_get<Display2D>(e);
    if (display) {
        if (UNLIKELY(display->program)) {
            programChanged = true;
            canvas_push_program(REF_RESOLVE(res_shader, display->program));
        }
        if (LIKELY(display->draw)) {
            canvas.matrix[0] = worldTransform->matrix;
            canvas.color[0] = worldTransform->color;
            display->draw(e);
        }
        if(UNLIKELY(display->callback)) {
            canvas.matrix[0] = worldTransform->matrix;
            canvas.color[0] = worldTransform->color;
            display->callback(e);
        }
    }

    auto it = get_first_child(e);
    while (it.id) {
        const auto& child = ecs::get<Node>(it);
        if (!(child.flags & NODE_HIDDEN)) {
            const auto* childWorldTransform = ecs::try_get<WorldTransform2D>(it);
            if (childWorldTransform) {
                worldTransform = childWorldTransform;
            }
            if (worldTransform->color.scale.a > 0) {
                draw(it, worldTransform);
            }
        }
        it = child.sibling_next;
    }

    if (bounds && (bounds->flags & BOUNDS_2D_SCISSORS)) {
        canvas_pop_scissors();
    }
    if (programChanged) {
        canvas_restore_program();
    }
}

void RenderSystem2D::drawStack(entity_t e) {
    EK_ASSERT(is_entity(e));

    auto* bounds = ecs::try_get<Bounds2D>(e);
    if (bounds) {
        const auto* camera = Camera2D::getCurrentRenderingCamera();
        auto rc = bounds->getScreenRect(camera->worldToScreenMatrix, canvas.matrix[0]);
        if (Camera2D::getCurrentRenderingCamera()->occlusionEnabled) {
            if (!rect_overlaps(rc, camera->screenRect) || !rect_overlaps(rc, canvas.scissors[0])) {
                // discard
                return;
            }
        }
        if (bounds->flags & BOUNDS_2D_SCISSORS) {
            canvas_push_scissors(rc);
            //draw2d::push_scissors(scissors->world_rect(transform->worldMatrix));
        }
    }

    bool programChanged = false;
    auto* display = ecs::try_get<Display2D>(e);
    if (display) {
        if (display->program) {
            programChanged = true;
            canvas_push_program(REF_RESOLVE(res_shader, display->program));
        }
        if (display->draw) {
            display->draw(e);
        }
        if (display->callback) {
            display->callback(e);
        }
    }

    auto it = get_first_child(e);
    while (it.id) {
        const auto& child = ecs::get<Node>(it);
        if (!(child.flags & NODE_HIDDEN)) {
            const auto* childTransform = ecs::try_get<Transform2D>(it);
            if (childTransform) {
                canvas_save_transform();
                canvas_concat_matrix(childTransform->matrix);
                canvas_concat_color(childTransform->color);
            }
            if (canvas.color[0].scale.a != 0) {
                drawStack(it);
            }
            if (childTransform) {
                canvas_restore_transform();
            }
        }
        it = child.sibling_next;
    }

    if (bounds && (bounds->flags & BOUNDS_2D_SCISSORS)) {
        canvas_pop_scissors();
    }
    if (programChanged) {
        canvas_restore_program();
    }
}
}