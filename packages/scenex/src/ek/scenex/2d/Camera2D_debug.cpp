#include <ek/canvas.h>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/util/ServiceLocator.hpp>
#include <ek/time.h>
#include <ek/scenex/base/Node.hpp>
#include "Camera2D.hpp"
#include "Viewport.hpp"
#include "Transform2D.hpp"
#include "Display2D.hpp"
#include <ek/scenex/base/Script.hpp>

namespace ek {

void debugDrawPointer(Camera2D& camera) {
    auto& im = Locator::ref<InteractionSystem>();
    auto v = im.pointerScreenPosition_;
    v = vec2_transform(v, camera.screenToWorldMatrix);
    float t = TimeLayer::Root->total;
    canvas_set_empty_image();
    if (im.pointerDown_) {
        canvas_fill_circle(circle(v.x, v.y, 12 + 2 * sinf(t * 8)), 0x00FFFF00_argb, 0x77FF0000_argb, 10);
    }
    else {
        canvas_fill_circle(circle(v.x, v.y, 12 + 2 * sinf(t)), 0x0_argb, 0x77FFFFFF_argb, 10);
    }
}

void drawBox(const rect_t rc, const mat3x2_t m, argb32_t color1, argb32_t color2,
             bool cross = true, argb32_t fillColor = 0_argb) {

    canvas_set_empty_image();
    if (fillColor != argb32_t::zero) {
        canvas_save_matrix();
        canvas.matrix[0] = m;
        canvas_fill_rect(rc, fillColor);
        canvas_restore_matrix();
    }
    const auto bb = brect_from_rect(rc);
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
    auto& im = Locator::ref<InteractionSystem>();
    auto target = im.getHitTarget();
    if (!target) {
        return;
    }
    mat3x2_t matrix = mat3x2_identity();
    auto* worldTransform = findComponentInParent<WorldTransform2D>(target);
    if (worldTransform) {
        matrix = worldTransform->matrix;
    }
    auto* display = target.tryGet<Display2D>();
    if (display && display->drawable) {
        drawBox(display->getBounds(), matrix, 0xFF000000_argb, argb32_t::one, true);
    }
    auto* bounds = target.tryGet<Bounds2D>();
    if (bounds) {
        if (bounds->hitArea) {
            drawBox(bounds->rect, matrix, 0xFF99FF00_argb, argb32_t::one, false);
        }
        if (bounds->scissors) {
            drawBox(bounds->rect, matrix, 0xFFFFFF00_argb, argb32_t::one, false);
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
        callback(*component, parentTransform);
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
            [](Display2D& display, const WorldTransform2D* transform) {
                if (display.drawable) {
                    canvas_fill_rect(display.getBounds(), 0x33FFFFFF_argb);
                    drawBox(display.getBounds(), transform->matrix, argb32_t::black, argb32_t::one,
                            false, 0x33FFFFFF_argb);
                } else {
                    const auto v = vec2_transform(vec2(0, 0), transform->matrix);
                    canvas_fill_circle(circle(v.x, v.y, 20.0f), 0xFFFF0000_argb,
                                        0x77FF0000_argb, 7);
                }
            });

    traverseVisibleNodes<Bounds2D>(
            camera.root.ent(),
            nullptr,
            [](Bounds2D& bounds, const WorldTransform2D* transform) {
                if (bounds.scissors) {
                    drawBox(bounds.rect, transform->matrix, 0xFFFFFF00_argb, argb32_t::one, true,
                            0x55FFFF00_argb);
                } else if (bounds.hitArea) {
                    drawBox(bounds.rect, transform->matrix, 0xFF00FF00_argb, argb32_t::one, true,
                            0x5500FF00_argb);
                } else if (bounds.culling) {
                    drawBox(bounds.rect, transform->matrix, 0xFF00FFFF_argb, argb32_t::one, true,
                            0x5500FFFF_argb);
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
                                   [cameraRect](const Bounds2D& bounds, const WorldTransform2D* transform) {
                                       const rect_t worldRect = rect_transform(bounds.rect,transform->matrix);
                                       const bool occluded = !rect_overlaps(worldRect, cameraRect);
                                       const auto worldColor = occluded ? 0x77FF0000_argb : 0x7700FF00_argb;
                                       drawBox(worldRect, mat3x2_identity(), worldColor, worldColor, false);
                                       const auto boundsColor = occluded ? 0x77770000_argb : 0x77007700_argb;
                                       drawBox(bounds.rect, transform->matrix, boundsColor, boundsColor, false);
                                   });
    canvas_restore_transform();
}

void debugCameraGizmo(Camera2D& camera) {
    auto rc = rect_expand(camera.worldRect, -10.0f);
    drawBox(rc, mat3x2_identity(), 0xFFFFFFFF_argb, 0xFF000000_argb);

    {
        // it's not correct because:
        // - viewport's viewport is applied
        // - viewport's MVP matrix
        // TODO: make display-space debug drawing for all viewports via inspector
        const auto& vp = camera.viewportNode.get().get<Viewport>();
        canvas_fill_rect(vp.output.safeRect, 0x77FF00FF_argb);
        canvas_quad_color(0, 0, vp.options.baseResolution.x, vp.options.baseResolution.y, 0x7700FFFF_argb);
    }
    auto v = camera.screenRect.position + camera.relativeOrigin * camera.screenRect.size;
    v = vec2_transform(v, camera.screenToWorldMatrix);
    canvas_fill_circle(circle(v.x, v.y, 10.0f), 0x00FFFFFF_argb, 0x44FFFFFF_argb, 7);
    canvas_line_ex(v - vec2(20, 0), v + vec2(20, 0), 0xFF000000_argb, 0xFFFFFFFF_argb, 1, 3);
    canvas_line_ex(v - vec2(0, 20), v + vec2(0, 20), 0xFF000000_argb, 0xFFFFFFFF_argb, 3, 1);
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
    if (camera.debugDrawScriptGizmo) {
        for (auto e : ecs::view<ScriptHolder>()) {
            auto* worldTransform = findComponentInParent<WorldTransform2D>(e);
            if (worldTransform) {
                canvas.matrix[0] = worldTransform->matrix;
                canvas.color[0] = worldTransform->color;
                for (auto& script : e.get<ScriptHolder>().list) {
                    script->gui_gizmo();
                }
            }
        }
    }
}

}