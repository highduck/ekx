#include <ek/scenex/app/basic_application.hpp>
#include <ek/math/bounds_builder.hpp>
#include <ek/math/matrix_inverse.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/components/script.hpp>
#include "Camera2D.hpp"
#include "Transform2D.hpp"
#include "Display2D.hpp"

namespace ek {

void drawEntity(ecs::entity e, const Transform2D* transform);

matrix_2d Camera2D::getMatrix(ecs::entity root_, float scale) const {
    auto screen = screenRect;
    auto m = root_.get<Transform2D>().worldMatrix;
    float invScale = 1.0f / (scale * contentScale);
    m.scale(invScale, invScale).translate(-screen.position - relativeOrigin * screen.size);
    return m;
}

static std::vector<ecs::entity> activeCameras;
static const Camera2D* currentCamera = nullptr;
static int currentLayerMask = 0xFF;

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

static mat4f tmpProj{};

void Camera2D::render() {

    draw2d::state.save_transform()
            .save_mvp()
            .saveScissors();

    tmpProj = draw2d::state.mvp;

    for (auto e : activeCameras) {
        auto& camera = e.get<Camera2D>();

        // set current
        currentCamera = &camera;
        currentLayerMask = camera.layerMask;

        // apply scissors
        draw2d::state.setScissors(camera.screenRect);

        // apply MVP
        draw2d::state.set_mvp(tmpProj * camera.inverseMatrix);

        // if (camera.clearColorEnabled) {
        //     drawer.invalidateForce();
        //     engine.graphics.clear(camera.clearColor);
        // }
        draw2d::state.color = {};
        draw2d::state.matrix.set_identity();

        if (camera.clearColorEnabled) {
            draw2d::state.set_empty_texture();
            draw2d::quad(camera.worldRect, argb32_t{camera.clearColor});
        }

        drawEntity(camera.root, camera.root.tryGet<Transform2D>());

#ifndef NDEBUG
        drawGizmo(camera);
#endif

        draw2d::flush_batcher();
        // drawer.batcher.state.invalidate();
    }

    draw2d::state.pop_scissors()
            .restore_mvp()
            .restore_transform();
}

Camera2D::Camera2D(ecs::entity root_) :
        root{root_} {

}

void drawEntity(ecs::entity e, const Transform2D* transform) {
    assert(e.valid());

    // todo:
//    if (process_node_filters(e)) {
//        return;
//    }

    auto* scissors = e.tryGet<scissors_2d>();
    if (scissors) {
        draw2d::state.push_scissors(scissors->world_rect(transform->worldMatrix));
    }

    auto* display = e.tryGet<Display2D>();
    if (display && display->drawable) {
        draw2d::state.matrix = transform->worldMatrix;
        draw2d::state.color = transform->worldColor;
        display->drawable->draw();
    }

    auto* scripts = e.tryGet<script_holder>();
    if (scripts) {
        draw2d::state.matrix = transform->worldMatrix;
        draw2d::state.color = transform->worldColor;
        for (auto& script : scripts->list) {
            if (script) {
                script->draw();
            }
        }
    }

    auto it = e.get<Node>().child_first;
    while (it) {
        const auto& child = it.get<Node>();
        if (child.visible() && (child.layersMask() & currentLayerMask) != 0) {
            const auto* childTransform = it.tryGet<Transform2D>();
            if (childTransform) {
                transform = childTransform;
            }
            if (transform->color.scale.a > 0) {
                drawEntity(it, transform);
            }
        }
        it = child.sibling_next;
    }

    if (scissors) {
        draw2d::state.pop_scissors();
    }
}

}