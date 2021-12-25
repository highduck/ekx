#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/util/ServiceLocator.hpp>
#include <ek/time.h>
#include <ek/scenex/base/Node.hpp>
#include "Camera2D.hpp"
#include "Viewport.hpp"
#include "Transform2D.hpp"
#include "Display2D.hpp"
#include <ek/scenex/base/Script.hpp>
#include <ek/math/BoundsBuilder.hpp>

namespace ek {

void debugDrawPointer(Camera2D& camera) {
    auto& im = Locator::ref<InteractionSystem>();
    auto ptr = im.pointerScreenPosition_;
    auto v = camera.screenToWorldMatrix.transform(ptr.x, ptr.y);
    float t = TimeLayer::Root->total;
    draw2d::state.set_empty_image();
    if (im.pointerDown_) {
        draw2d::fill_circle({v.x, v.y, 12 + 2 * sinf(t * 8)}, 0x00FFFF00_argb, 0x77FF0000_argb, 10);
    }
    else {
        draw2d::fill_circle({v.x, v.y, 12 + 2 * sinf(t)}, 0x0_argb, 0x77FFFFFF_argb, 10);
    }
}

void drawBox(const Rect2f& rc, const Matrix3x2f& m, argb32_t color1, argb32_t color2,
             bool cross = true, argb32_t fillColor = 0_argb) {

    draw2d::state.set_empty_image();
    if (fillColor != argb32_t::zero) {
        draw2d::state.save_matrix();
        draw2d::state.matrix = m;
        draw2d::quad(rc, fillColor);
        draw2d::state.restore_matrix();
    }
    auto v1 = m.transform(rc.x, rc.y);
    auto v2 = m.transform(rc.right(), rc.y);
    auto v3 = m.transform(rc.right(), rc.bottom());
    auto v4 = m.transform(rc.x, rc.bottom());
    draw2d::line(v1, v2, color1, color2, 2, 1);
    draw2d::line(v2, v3, color1, color2, 1, 2);
    draw2d::line(v3, v4, color1, color2, 2, 1);
    draw2d::line(v4, v1, color1, color2, 1, 2);
    if (cross) {
        draw2d::line(v1, v3, color1, color2, 2, 2);
        draw2d::line(v2, v4, color1, color2, 1, 1);
    }
}

void debugDrawHitTarget(Camera2D& camera) {
    auto& im = Locator::ref<InteractionSystem>();
    auto target = im.getHitTarget();
    if (!target) {
        return;
    }
    Matrix3x2f matrix{};
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

    draw2d::state.save_transform();
    draw2d::state.color = {};
    draw2d::state.set_empty_image();

    traverseVisibleNodes<Display2D>(
            camera.root.ent(),
            nullptr,
            [](Display2D& display, const WorldTransform2D* transform) {
                if (display.drawable) {
                    draw2d::quad(display.getBounds(), 0x33FFFFFF_argb);
                    drawBox(display.getBounds(), transform->matrix, argb32_t::black, argb32_t::one,
                            false, 0x33FFFFFF_argb);
                } else {
                    draw2d::fill_circle({transform->matrix.transform(0, 0), 20.0f}, 0xFFFF0000_argb,
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

    draw2d::state.restore_transform();
}

void drawOcclusion(Camera2D& camera) {

    draw2d::state.save_transform();
    draw2d::state.color = {};
    draw2d::state.set_empty_image();
    auto cameraRect = camera.worldRect;
    traverseVisibleNodes<Bounds2D>(camera.root.ent(), nullptr,
                                   [cameraRect](const Bounds2D& bounds, const WorldTransform2D* transform) {
                                       const auto worldRect = BoundsBuilder2f::transform(bounds.rect,
                                                                                         transform->matrix);
                                       const bool occluded = !worldRect.overlaps(cameraRect);
                                       const auto worldColor = occluded ? 0x77FF0000_argb : 0x7700FF00_argb;
                                       drawBox(worldRect, Matrix3x2f{}, worldColor, worldColor, false);
                                       const auto boundsColor = occluded ? 0x77770000_argb : 0x77007700_argb;
                                       drawBox(bounds.rect, transform->matrix, boundsColor, boundsColor, false);
                                   });
    draw2d::state.restore_transform();
}

void debugCameraGizmo(Camera2D& camera) {
    auto rc = expand(camera.worldRect, -10.0f);
    drawBox(rc, Matrix3x2f{}, 0xFFFFFFFF_argb, 0xFF000000_argb);

    {
        // it's not correct because:
        // - viewport's viewport is applied
        // - viewport's MVP matrix
        // TODO: make display-space debug drawing for all viewports via inspector
        const auto& vp = camera.viewportNode.get().get<Viewport>();
        draw2d::quad(vp.output.safeRect, 0x77FF00FF_argb);
        draw2d::quad(0.0f, 0.0f, vp.options.baseResolution.x, vp.options.baseResolution.y, 0x7700FFFF_argb);
    }
    auto v = camera.screenToWorldMatrix.transform(camera.screenRect.relative(camera.relativeOrigin));

    draw2d::fill_circle({v, 10.0f}, 0x00FFFFFF_argb, 0x44FFFFFF_argb, 7);
    draw2d::line(v - Vec2f{20, 0}, v + Vec2f{20, 0}, 0xFF000000_argb, 0xFFFFFFFF_argb, 1, 3);
    draw2d::line(v - Vec2f{0, 20}, v + Vec2f{0, 20}, 0xFF000000_argb, 0xFFFFFFFF_argb, 3, 1);
}

void Camera2D::drawGizmo(Camera2D& camera) {
    draw2d::state.matrix.set_identity();
    draw2d::state.color = {};

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
        draw2d::state.matrix.set_identity();
        draw2d::state.color = {};
        debugCameraGizmo(camera);
    }
    if (camera.debugDrawScriptGizmo) {
        for (auto e : ecs::view<ScriptHolder>()) {
            auto* worldTransform = findComponentInParent<WorldTransform2D>(e);
            if (worldTransform) {
                draw2d::state.matrix = worldTransform->matrix;
                draw2d::state.color = worldTransform->color;
                for (auto& script : e.get<ScriptHolder>().list) {
                    script->gui_gizmo();
                }
            }
        }
    }
}

}