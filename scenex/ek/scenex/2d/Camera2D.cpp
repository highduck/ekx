#include "Camera2D.hpp"
#include "Transform2D.hpp"
#include "RenderSystem2D.hpp"

#include <ek/math/bounds_builder.hpp>
#include <ek/math/matrix_inverse.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ek/scenex/app/basic_application.hpp>

namespace ek {

void drawEntity(ecs::entity e, const Transform2D* transform);

ecs::entity Camera2D::Main{};

matrix_2d Camera2D::getMatrix(ecs::entity root_, float scale) const {
    auto screen = screenRect;
    auto m = root_.get<Transform2D>().worldMatrix;
    float invScale = 1.0f / (scale * contentScale);
    m.scale(invScale, invScale).translate(-screen.position - relativeOrigin * screen.size);
    return m;
}

static std::vector<ecs::entity> activeCameras{};
static const Camera2D* currentRenderingCamera = nullptr;
static int currentLayerMask = 0xFF;

std::vector<ecs::entity>& Camera2D::getCameraQueue() {
    return activeCameras;
}

void Camera2D::updateQueue() {
    auto& app = resolve<basic_application>();
    const auto contentScale = app.scale_factor;
    const float2 fbSize{ek::app::g_app.drawable_size};

    activeCameras.clear();
    for (auto e : ecs::view<Camera2D>()) {
        auto& camera = e.get<Camera2D>();
        if (!camera.enabled) {
            continue;
        }

        if (camera.syncContentScale) {
            camera.contentScale = contentScale;
        }

        camera.screenRect = camera.viewport * fbSize;

        camera.matrix = camera.getMatrix(e, 1.0f);

        camera.worldRect = bounds_builder_2f::transform(camera.screenRect, camera.matrix);

        camera.matrix = camera.getMatrix(e, camera.debugDrawScale);

        camera.inverseMatrix = camera.matrix;
        if (camera.inverseMatrix.inverse()) {
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

        draw2d::begin(rect_i{camera.screenRect}, camera.inverseMatrix);
        if (camera.clearColorEnabled) {
            draw2d::current().pushProgram("draw2d_color");
            draw2d::current().color = ColorMod32{argb32_t{camera.clearColor}, argb32_t{camera.clearColor2}};
            draw2d::quad(camera.worldRect);
            draw2d::current().color = {};
            draw2d::current().restoreProgram();
        }

        RenderSystem2D::draw(camera.root, camera.root.tryGet<Transform2D>());

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