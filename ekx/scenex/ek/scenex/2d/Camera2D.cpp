#include "Camera2D.hpp"
#include "Transform2D.hpp"
#include "RenderSystem2D.hpp"
#include "Viewport.hpp"

#include <ek/math/bounds_builder.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/app/basic_application.hpp>

namespace ek {

void drawEntity(ecs::EntityApi e, const Transform2D* transform);

ecs::EntityApi Camera2D::Main{};

matrix_2d
Camera2D::getMatrix(ecs::EntityApi root_, float scale, const float2& screenOffset, const float2& screenSize) const {
//    auto screen = screenRect;
    auto m = root_.get<WorldTransform2D>().matrix;
    float invScale = 1.0f / (scale * contentScale);
//    m.scale(invScale, invScale).translate( -screen.position - relativeOrigin * screen.size - screenOffset);
    m.translate(-relativeOrigin * screenSize).scale(invScale, invScale).translate( - screenOffset);
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

    for (auto e : ecs::view<Camera2D>()) {
        auto& camera = e.get<Camera2D>();
        const auto& vp = camera.viewportNode.get().get<Viewport>();
        if (!camera.enabled) {
            continue;
        }

        // maybe we need region from not 0,0 started input rect
        camera.screenRect = vp.output.screenRect;
        camera.screenToWorldMatrix = camera.getMatrix(e, vp.output.scale, vp.output.offset, vp.options.baseResolution);
        camera.worldRect = bounds_builder_2f::transform(camera.screenRect, camera.screenToWorldMatrix);

        camera.worldToScreenMatrix = camera.screenToWorldMatrix;
        if (camera.worldToScreenMatrix.inverse()) {
            activeCameras.push_back(ecs::EntityRef{e});
        } else {
            // please debug camera setup
            //EK_ASSERT(false);
        }
    }
    std::sort(activeCameras.begin(), activeCameras.end(), [](ecs::EntityRef a, ecs::EntityRef b) -> bool {
        return a.get().get<Camera2D>().order < b.get().get<Camera2D>().order;
    });
}


void Camera2D::render() {
    for (auto e : activeCameras) {
        if(!e.valid()) {
            continue;
        }
        auto& camera = e.get().get<Camera2D>();
        if (camera.screenRect.empty()) {
            continue;
        }

        // set current
        currentRenderingCamera = &camera;
        currentLayerMask = camera.layerMask;

        sg_push_debug_group("Camera");
        draw2d::begin(camera.screenRect, camera.worldToScreenMatrix);
        sg_apply_viewportf(camera.screenRect.x, camera.screenRect.y, camera.screenRect.width, camera.screenRect.height,
                           true);
        if (camera.clearColorEnabled) {
            draw2d::state.pushProgram("draw2d_color");
            draw2d::state.color = ColorMod32{argb32_t{camera.clearColor}, argb32_t{camera.clearColor2}};
            draw2d::quad(camera.worldRect);
            draw2d::state.color = {};
            draw2d::state.restoreProgram();
        }

        if(camera.root.valid() && camera.root.get().get_or_default<Node>().visible()) {
            RenderSystem2D::draw(ecs::the_world, camera.root.index(), camera.root.get().tryGet<WorldTransform2D>());
        }

#ifndef NDEBUG
//        draw2d::begin(camera.screenRect, camera.worldToScreenMatrix);
        drawGizmo(camera);
//        sg_apply_viewportf(camera.screenRect.x, camera.screenRect.y, camera.screenRect.width, camera.screenRect.height, true);
#endif

        draw2d::end();
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