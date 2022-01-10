#include "Camera2D.hpp"
#include "Transform2D.hpp"
#include "RenderSystem2D.hpp"
#include "Viewport.hpp"

#include <ek/canvas.h>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

void drawEntity(ecs::EntityApi e, const Transform2D* transform);

ecs::EntityApi Camera2D::Main{};

mat3x2_t
Camera2D::getMatrix(ecs::EntityApi root_, float scale, vec2_t screenOffset, vec2_t screenSize) const {
    const float inv_scale = 1.0f / (scale * contentScale);
    mat3x2_t m = root_.get<WorldTransform2D>().matrix;
    mat3x2_translate(&m, -relativeOrigin * screenSize);
    mat3x2_scale(&m, vec2(inv_scale, inv_scale));
    mat3x2_translate(&m, -screenOffset);
    return m;
}

static FixedArray<ecs::EntityRef, Camera2D::MaxCount> activeCameras{};
static const Camera2D* currentRenderingCamera = nullptr;
static int currentLayerMask = 0xFF;

FixedArray<ecs::EntityRef, Camera2D::MaxCount>& Camera2D::getCameraQueue() {
    return activeCameras;
}

void Camera2D::updateQueue() {
    activeCameras.clear();

    for (auto e: ecs::view<Camera2D>()) {
        auto& camera = e.get<Camera2D>();
        const auto* vp = camera.viewportNode.get().tryGet<Viewport>();
        if (!camera.enabled || !vp) {
            continue;
        }

        // maybe we need region from not 0,0 started input rect
        camera.screenRect = vp->output.screenRect;
        camera.screenToWorldMatrix = camera.getMatrix(e, vp->output.scale, vp->output.offset,
                                                      vp->options.baseResolution);
        camera.worldRect = rect_transform(camera.screenRect, camera.screenToWorldMatrix);

        camera.worldToScreenMatrix = camera.screenToWorldMatrix;
        if (mat3x2_inverse(&camera.worldToScreenMatrix)) {
            activeCameras.push_back(ecs::EntityRef{e});
        } else {
            // please debug camera setup
            EK_ASSERT(false);
        }
    }
    qsort(activeCameras.data(), activeCameras.size(), sizeof(ecs::EntityRef), [](void const* _a, void const* _b) {
        auto* a = (ecs::EntityRef*) _a;
        auto* b = (ecs::EntityRef*) _b;
        return a->get().get<Camera2D>().order - b->get().get<Camera2D>().order;
    });
}

void Camera2D::render() {
    for (auto e: activeCameras) {
        if (!e.valid()) {
            continue;
        }
        auto& camera = e.get().get<Camera2D>();
        if (rect_is_empty(camera.screenRect)) {
            continue;
        }

        // set current
        currentRenderingCamera = &camera;
        currentLayerMask = camera.layerMask;

        sg_push_debug_group("Camera");
        canvas_begin_ex(camera.screenRect, camera.worldToScreenMatrix, {0}, {0});
        sg_apply_viewportf(camera.screenRect.x, camera.screenRect.y, camera.screenRect.w, camera.screenRect.h,
                           true);
        if (camera.clearColorEnabled) {
            canvas_push_program(canvas.shader_solid_color);
            canvas.color[0] = {color_vec4(camera.clearColor), color_vec4(camera.clearColor2)};
            canvas_fill_rect(camera.worldRect, COLOR_WHITE);
            canvas.color[0] = color2_identity();
            canvas_restore_program();
        }

        if (camera.root.valid() && camera.root.get().get_or_default<Node>().visible()) {
            RenderSystem2D::draw(ecs::the_world, camera.root.index(), camera.root.get().tryGet<WorldTransform2D>());
        }

#ifndef NDEBUG
        //        draw2d::begin(camera.screenRect, camera.worldToScreenMatrix);
                drawGizmo(camera);
        //        sg_apply_viewportf(camera.screenRect.x, camera.screenRect.y, camera.screenRect.width, camera.screenRect.height, true);
#endif

        canvas_end();
        sg_pop_debug_group();
    }
}

Camera2D::Camera2D(ecs::EntityApi root_) :
        root{root_} {

}

const Camera2D* Camera2D::getCurrentRenderingCamera() {
    return currentRenderingCamera;
}

}
