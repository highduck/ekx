#include "Camera2D.hpp"
#include "Transform2D.hpp"
#include "RenderSystem2D.hpp"
#include "Viewport.hpp"

#include <ek/math/bounds_builder.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/app/basic_application.hpp>

namespace ek {

void drawEntity(ecs::entity e, const Transform2D* transform);

ecs::entity Camera2D::Main{};

matrix_2d Camera2D::getMatrix(ecs::entity root_, float scale, const float2& screenOffset) const {
    auto screen = screenRect;
    auto m = root_.get<WorldTransform2D>().matrix;
    float invScale = 1.0f / (scale * contentScale);
    m.scale(invScale, invScale).translate(screen.position - relativeOrigin * screen.size-screenOffset);
    return m;
}

static std::vector<ecs::entity> activeCameras{};
static const Camera2D* currentRenderingCamera = nullptr;
static int currentLayerMask = 0xFF;

std::vector<ecs::entity>& Camera2D::getCameraQueue() {
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
        camera.screenRect = {
                vp.input.fullRect.position * camera.viewport.position,
                vp.input.fullRect.size * camera.viewport.size
        };

        camera.screenToWorldMatrix = camera.getMatrix(e, vp.output.scale, vp.output.offset);

        camera.worldRect = bounds_builder_2f::transform(camera.screenRect, camera.screenToWorldMatrix);

        camera.worldToScreenMatrix = camera.screenToWorldMatrix;
        if (camera.worldToScreenMatrix.inverse()) {
            activeCameras.push_back(e);
        } else {
            // please debug camera setup
            assert(false);
        }
    }
    std::sort(activeCameras.begin(), activeCameras.end(), [](ecs::entity a, ecs::entity b) -> bool {
        return a.get<Camera2D>().order < b.get<Camera2D>().order;
    });
}


void Camera2D::render() {
    for (auto e : activeCameras) {
        auto& camera = e.get<Camera2D>();
        // set current
        currentRenderingCamera = &camera;
        currentLayerMask = camera.layerMask;

        draw2d::begin(camera.screenRect, camera.worldToScreenMatrix);
        if (camera.clearColorEnabled) {
            draw2d::state.pushProgram("draw2d_color");
            draw2d::state.color = ColorMod32{argb32_t{camera.clearColor}, argb32_t{camera.clearColor2}};
            draw2d::quad(camera.worldRect);
            draw2d::state.color = {};
            draw2d::state.restoreProgram();
        }

        RenderSystem2D::draw(ecs::the_world, camera.root.index(), camera.root.get().tryGet<WorldTransform2D>());

#ifndef NDEBUG
        drawGizmo(camera);
#endif

        draw2d::end();
    }
}

Camera2D::Camera2D(ecs::entity root_) :
        root{root_} {

}

const Camera2D* Camera2D::getCurrentRenderingCamera() {
    return currentRenderingCamera;
}

}