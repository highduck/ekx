#include "Camera2D.hpp"
#include "Transform2D.hpp"
#include "RenderSystem2D.hpp"
#include "Viewport.hpp"

#include <ek/canvas.h>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

entity_t Camera2D::Main{};

mat3x2_t
Camera2D::getMatrix(entity_t root_, float scale, vec2_t screenOffset, vec2_t screenSize) const {
    const float inv_scale = 1.0f / (scale * contentScale);
    mat3x2_t m = ecs::get<WorldTransform2D>(root_).matrix;
    mat3x2_translate(&m, -relativeOrigin * screenSize);
    mat3x2_scale(&m, vec2(inv_scale, inv_scale));
    mat3x2_translate(&m, -screenOffset);
    return m;
}

static FixedArray<entity_t, Camera2D::MaxCount> activeCameras{};
static const Camera2D* currentRenderingCamera = nullptr;
static int currentLayerMask = 0xFF;

FixedArray<entity_t, Camera2D::MaxCount>& Camera2D::getCameraQueue() {
    return activeCameras;
}

void update_camera2d_queue() {
    activeCameras.clear();

    for (auto e: ecs::view<Camera2D>()) {
        auto& camera = ecs::get<Camera2D>(e);
        const auto* vp = ecs::try_get<Viewport>(camera.viewportNode);
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
            activeCameras.push_back(e);
        } else {
            // please debug camera setup
            EK_ASSERT(false);
        }
    }
    qsort(activeCameras.data(), activeCameras.size(), sizeof(entity_t), [](void const* _a, void const* _b) {
        auto* a = (entity_t*) _a;
        auto* b = (entity_t*) _b;
        return ecs::get<Camera2D>(*a).order - ecs::get<Camera2D>(*b).order;
    });
}

void render_camera2d_queue() {
    for (auto e: activeCameras) {
        if (!is_entity(e)) {
            continue;
        }
        auto& camera = ecs::get<Camera2D>(e);
        if (rect_is_empty(camera.screenRect)) {
            continue;
        }

        // set current
        currentRenderingCamera = &camera;

        sg_push_debug_group("Camera");
        canvas_begin_ex(camera.screenRect, camera.worldToScreenMatrix, {0}, {0});
        sg_apply_viewportf(camera.screenRect.x, camera.screenRect.y, camera.screenRect.w, camera.screenRect.h,
                           true);
        if (camera.clearColorEnabled) {
            canvas_push_program(res_shader.data[R_SHADER_SOLID_COLOR]);
            canvas.color[0] = {color_vec4(camera.clearColor), color_vec4(camera.clearColor2)};
            canvas_fill_rect(camera.worldRect, COLOR_WHITE);
            canvas.color[0] = color2_identity();
            canvas_restore_program();
        }

        entity_t camera_root = camera.root;
        if (is_entity(camera_root) && is_visible(camera_root)) {
            RenderSystem2D::draw(camera_root, ecs::try_get<WorldTransform2D>(camera_root));
        }

#ifndef NDEBUG
        //        draw2d::begin(camera.screenRect, camera.worldToScreenMatrix);
        Camera2D::drawGizmo(camera);
        //        sg_apply_viewportf(camera.screenRect.x, camera.screenRect.y, camera.screenRect.width, camera.screenRect.height, true);
#endif

        canvas_end();
        sg_pop_debug_group();
    }
}

const Camera2D* Camera2D::getCurrentRenderingCamera() {
    return currentRenderingCamera;
}

}
