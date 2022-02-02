#include <ek/canvas.h>
#include <ek/scenex/InteractionSystem.hpp>

#include <ek/scenex/base/Node.hpp>
#include <ekx/app/time_layers.h>
#include "Camera2D.hpp"
#include "Viewport.hpp"
#include "Transform2D.hpp"
#include "Display2D.hpp"

namespace ek {

void debugDrawPointer(Camera2D& camera) {
    auto* im = &g_interaction_system;
    auto v = im->pointerScreenPosition_;
    v = vec2_transform(v, camera.screenToWorldMatrix);
    float t = g_time_layers[TIME_LAYER_ROOT].total;
    canvas_set_empty_image();
    if (im->pointerDown_) {
        canvas_fill_circle(vec3_v(v, 12 + 2 * sinf(t * 8)), ARGB(0x00FFFF00), ARGB(0x77FF0000), 10);
    }
    else {
        canvas_fill_circle(vec3_v(v, 12 + 2 * sinf(t)), ARGB(0x0), ARGB(0x77FFFFFF), 10);
    }
}

void drawBox(const rect_t rc, const mat3x2_t m, color_t color1, color_t color2,
             bool cross = true, color_t fillColor = COLOR_ZERO) {

    canvas_set_empty_image();
    if (fillColor.value) {
        canvas_save_matrix();
        canvas.matrix[0] = m;
        canvas_fill_rect(rc, fillColor);
        canvas_restore_matrix();
    }
    const auto bb = aabb2_from_rect(rc);
    const auto v1 = vec2_transform(bb.min, m);
    const auto v2 = vec2_transform(vec2(bb.x1, bb.y0), m);
    const auto v3 = vec2_transform(bb.max, m);
    const auto v4 = vec2_transform(vec2(bb.x0, bb.y1), m);
    canvas_line_ex(v1, v2, color1, color2, 2, 1);
    canvas_line_ex(v2, v3, color1, color2, 1, 2);
    canvas_line_ex(v3, v4, color1, color2, 2, 1);
    canvas_line_ex(v4, v1, color1, color2, 1, 2);
    if (cross) {
        canvas_line_ex(v1, v3, color1, color2, 2, 2);
        canvas_line_ex(v2, v4, color1, color2, 1, 1);
    }
}

void debugDrawHitTarget(Camera2D& camera) {
    auto target = g_interaction_system.getHitTarget();
    if (!target) {
        return;
    }
    mat3x2_t matrix = mat3x2_identity();
    auto* worldTransform = findComponentInParent<WorldTransform2D>(target);
    if (worldTransform) {
        matrix = worldTransform->matrix;
    }
    auto* display = target.tryGet<Display2D>();
    if (display && display->get_bounds) {
        drawBox(display->get_bounds(target.index), matrix, COLOR_BLACK, COLOR_WHITE, true);
    }
    auto* bounds = target.tryGet<Bounds2D>();
    if (bounds) {
        if (bounds->flags & BOUNDS_2D_HIT_AREA) {
            drawBox(bounds->rect, matrix, RGB(0x99FF00), COLOR_WHITE, false);
        }
        if (bounds->flags & BOUNDS_2D_SCISSORS) {
            drawBox(bounds->rect, matrix, RGB(0xFFFF00), COLOR_WHITE, false);
        }
    }
}

template<typename Comp, typename Func>
void traverseVisibleNodes(ecs::EntityApi e, const WorldTransform2D* parentTransform, Func callback) {
    if (!e.get<Node>().visible()) {
        return;
    }

    auto* transform = e.tryGet<WorldTransform2D>();
    if (transform) {
        if (transform->color.scale.a <= 0.0) {
            return;
        }
        parentTransform = transform;
    }

    Comp* component = e.tryGet<Comp>();
    if (component) {
        callback(e, *component, parentTransform);
    }

    auto it = e.get<Node>().child_first;
    while (it) {
        traverseVisibleNodes<Comp, Func>(it, parentTransform, callback);
        it = it.get<Node>().sibling_next;
    }
}

void drawFills(Camera2D& camera) {

    canvas_save_transform();
    canvas.color[0] = color2_identity();
    canvas_set_empty_image();

    traverseVisibleNodes<Display2D>(
            camera.root.ent(),
            nullptr,
            [](ecs::EntityApi e, Display2D& display, const WorldTransform2D* transform) {
                if (display.get_bounds) {
                    auto rc = display.get_bounds(e.index);
                    canvas_fill_rect(rc, ARGB(0x33FFFFFF));
                    drawBox(rc, transform->matrix, COLOR_BLACK, COLOR_WHITE,
                            false, ARGB(0x33FFFFFF));
                } else {
                    const auto v = vec2_transform(vec2(0, 0), transform->matrix);
                    canvas_fill_circle(vec3_v(v, 20.0f), ARGB(0xFFFF0000),
                                        ARGB(0x77FF0000), 7);
                }
            });

    traverseVisibleNodes<Bounds2D>(
            camera.root.ent(),
            nullptr,
            [](ecs::EntityApi e, Bounds2D& bounds, const WorldTransform2D* transform) {
                if (bounds.flags & BOUNDS_2D_SCISSORS) {
                    drawBox(bounds.rect, transform->matrix, ARGB(0xFFFFFF00), COLOR_WHITE, true,
                            ARGB(0x55FFFF00));
                } else if (bounds.flags & BOUNDS_2D_HIT_AREA) {
                    drawBox(bounds.rect, transform->matrix, ARGB(0xFF00FF00), COLOR_WHITE, true,
                            ARGB(0x5500FF00));
                } else if (bounds.flags & BOUNDS_2D_CULL) {
                    drawBox(bounds.rect, transform->matrix, ARGB(0xFF00FFFF), COLOR_WHITE, true,
                            ARGB(0x5500FFFF));
                }
            });

    canvas_restore_transform();
}

void drawOcclusion(Camera2D& camera) {

    canvas_save_transform();
    canvas.color[0] = color2_identity();
    canvas_set_empty_image();
    auto cameraRect = camera.worldRect;
    traverseVisibleNodes<Bounds2D>(camera.root.ent(), nullptr,
                                   [cameraRect](ecs::EntityApi e, const Bounds2D& bounds, const WorldTransform2D* transform) {
                                       const rect_t worldRect = rect_transform(bounds.rect,transform->matrix);
                                       const bool occluded = !rect_overlaps(worldRect, cameraRect);
                                       const auto worldColor = occluded ? ARGB(0x77FF0000) : ARGB(0x7700FF00);
                                       drawBox(worldRect, mat3x2_identity(), worldColor, worldColor, false);
                                       const auto boundsColor = occluded ? ARGB(0x77770000) : ARGB(0x77007700);
                                       drawBox(bounds.rect, transform->matrix, boundsColor, boundsColor, false);
                                   });
    canvas_restore_transform();
}

void debugCameraGizmo(Camera2D& camera) {
    auto rc = rect_expand(camera.worldRect, -10.0f);
    drawBox(rc, mat3x2_identity(), COLOR_WHITE, COLOR_BLACK);

    {
        // it's not correct because:
        // - viewport's viewport is applied
        // - viewport's MVP matrix
        // TODO: make display-space debug drawing for all viewports via inspector
        const auto& vp = camera.viewportNode.get().get<Viewport>();
        canvas_fill_rect(vp.output.safeRect, ARGB(0x77FF00FF));
        canvas_quad_color(0, 0, vp.options.baseResolution.x, vp.options.baseResolution.y, ARGB(0x7700FFFF));
    }
    auto v = camera.screenRect.position + camera.relativeOrigin * camera.screenRect.size;
    v = vec2_transform(v, camera.screenToWorldMatrix);
    canvas_fill_circle(vec3_v(v, 10.0f), ARGB(0x00FFFFFF), ARGB(0x44FFFFFF), 7);
    canvas_line_ex(v - vec2(20, 0), v + vec2(20, 0), COLOR_BLACK, COLOR_WHITE, 1, 3);
    canvas_line_ex(v - vec2(0, 20), v + vec2(0, 20), COLOR_BLACK, COLOR_WHITE, 3, 1);
}

void Camera2D::drawGizmo(Camera2D& camera) {
    canvas.matrix[0] = mat3x2_identity();
    canvas.color[0] = color2_identity();

    if (camera.debugVisibleBounds) {
        drawFills(camera);
    }
    if (camera.debugOcclusion) {
        drawOcclusion(camera);
    }
    if (camera.debugGizmoHitTarget) {
        debugDrawHitTarget(camera);
    }
    if (camera.debugGizmoPointer) {
        debugDrawPointer(camera);
    }
    if (camera.debugGizmoSelf) {
        canvas.matrix[0] = mat3x2_identity();
        canvas.color[0] = color2_identity();
        debugCameraGizmo(camera);
    }
}

}