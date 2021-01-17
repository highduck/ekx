#include "RenderSystem2D.hpp"
#include "Display2D.hpp"
#include "Transform2D.hpp"
#include "UglyFilter2D.hpp"
#include "Camera2D.hpp"

#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ek/math/bounds_builder.hpp>

namespace ek {

int RenderSystem2D::currentLayerMask = 0xFF;

void RenderSystem2D::draw(ecs::entity e, const WorldTransform2D* worldTransform) {
    assert(e.isAlive());

    auto* uglyFilter = e.tryGet<UglyFilter2D>();
    if (uglyFilter && uglyFilter->enabled && !uglyFilter->processing) {
        draw2d::state.matrix = worldTransform->matrix;
        draw2d::state.color = worldTransform->color;
        if (uglyFilter->pass(e)) {
            // discard
            return;
        }
    }

    auto* bounds = e.tryGet<Bounds2D>();
    if (bounds) {
        const auto* camera = Camera2D::getCurrentRenderingCamera();
        auto rc = bounds->getScreenRect(camera->inverseMatrix, worldTransform->matrix);
        if (Camera2D::getCurrentRenderingCamera()->occlusionEnabled) {
            if (!rc.overlaps(camera->screenRect) || !rc.overlaps(draw2d::state.scissors)) {
                // discard
                return;
            }
        }
        if (bounds->scissors) {
            draw2d::state.pushClipRect(rc);
        }
    }

    bool programChanged = false;
    auto* display = e.tryGet<Display2D>();
    if (display) {
        if (display->program) {
            programChanged = true;
            draw2d::state.saveProgram().setProgram(display->program.get());
        }
        if (display->drawable) {
            draw2d::state.matrix = worldTransform->matrix;
            draw2d::state.color = worldTransform->color;
            display->drawable->draw();
        }
    }

    auto it = e.get<Node>().child_first;
    while (it) {
        const auto& child = it.get<Node>();
        if (child.visible() && (child.layersMask() & currentLayerMask) != 0) {
            const auto* childWorldTransform = it.tryGet<WorldTransform2D>();
            if (childWorldTransform) {
                worldTransform = childWorldTransform;
            }
            if (worldTransform->color.scale.a > 0) {
                draw(it, worldTransform);
            }
        }
        it = child.sibling_next;
    }

    if (bounds && bounds->scissors) {
        draw2d::state.popClipRect();
    }
    if (programChanged) {
        draw2d::state.restoreProgram();
    }
}


void RenderSystem2D::drawStack(ecs::entity e) {
    assert(e.isAlive());

    auto* uglyFilter = e.tryGet<UglyFilter2D>();
    if (uglyFilter && uglyFilter->enabled && !uglyFilter->processing) {
        if (uglyFilter->pass(e)) {
            // discard
            return;
        }
    }

    auto* bounds = e.tryGet<Bounds2D>();
    if (bounds) {
        const auto* camera = Camera2D::getCurrentRenderingCamera();
        auto rc = bounds->getScreenRect(camera->inverseMatrix, draw2d::state.matrix);
        if (Camera2D::getCurrentRenderingCamera()->occlusionEnabled) {
            if (!rc.overlaps(camera->screenRect) || !rc.overlaps(draw2d::state.scissors)) {
                // discard
                return;
            }
        }
        if (bounds->scissors) {
            draw2d::state.pushClipRect(rc);
            //draw2d::state.push_scissors(scissors->world_rect(transform->worldMatrix));
        }
    }

    bool programChanged = false;
    auto* display = e.tryGet<Display2D>();
    if (display) {
        if (display->program) {
            programChanged = true;
            draw2d::state.saveProgram().setProgram(display->program.get());
        }
        if (display->drawable) {
            display->drawable->draw();
        }
    }

    auto it = e.get<Node>().child_first;
    while (it) {
        const auto& child = it.get<Node>();
        if (child.visible() && (child.layersMask() & currentLayerMask) != 0) {
            const auto* childTransform = it.tryGet<Transform2D>();
            if (childTransform) {
                draw2d::state.save_transform();
                draw2d::state.concat(childTransform->matrix);
                draw2d::state.concat(childTransform->color);
            }
            if (draw2d::state.color.scale.a > 0) {
                drawStack(it);
            }
            if (childTransform) {
                draw2d::state.restore_transform();
            }
        }
        it = child.sibling_next;
    }

    if (bounds && bounds->scissors) {
        draw2d::state.popClipRect();
    }
    if (programChanged) {
        draw2d::state.restoreProgram();
    }
}
}