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

void RenderSystem2D::draw(ecs::entity e, const Transform2D* transform) {
    assert(e.valid());

    auto* uglyFilter = e.tryGet<UglyFilter2D>();
    if (uglyFilter && uglyFilter->enabled && !uglyFilter->processing) {
        draw2d::current().matrix = transform->worldMatrix;
        draw2d::current().color = transform->worldColor;
        if (uglyFilter->pass(e)) {
            // discard
            return;
        }
    }

    auto* bounds = e.tryGet<Bounds2D>();
    if (bounds) {
        const auto* camera = Camera2D::getCurrentRenderingCamera();
        auto rc = bounds->getScreenRect(camera->inverseMatrix, transform->worldMatrix);
        if (Camera2D::getCurrentRenderingCamera()->occlusionEnabled) {
            if (!rc.overlaps(camera->screenRect) || !rc.overlaps(draw2d::current().scissors)) {
                // discard
                return;
            }
        }
        if (bounds->scissors) {
            draw2d::current().pushClipRect(rc);
        }
    }

    bool programChanged = false;
    auto* display = e.tryGet<Display2D>();
    if (display) {
        if (display->program) {
            programChanged = true;
            draw2d::current().saveProgram().setProgram(display->program.get());
        }
        if (display->drawable) {
            draw2d::current().matrix = transform->worldMatrix;
            draw2d::current().color = transform->worldColor;
            display->drawable->draw();
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
                draw(it, transform);
            }
        }
        it = child.sibling_next;
    }

    if (bounds && bounds->scissors) {
        draw2d::current().popClipRect();
    }
    if (programChanged) {
        draw2d::current().restoreProgram();
    }
}


void RenderSystem2D::drawStack(ecs::entity e) {
    assert(e.valid());

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
        auto rc = bounds->getScreenRect(camera->inverseMatrix, draw2d::current().matrix);
        if (Camera2D::getCurrentRenderingCamera()->occlusionEnabled) {
            if (!rc.overlaps(camera->screenRect) || !rc.overlaps(draw2d::current().scissors)) {
                // discard
                return;
            }
        }
        if (bounds->scissors) {
            draw2d::current().pushClipRect(rc);
            //draw2d::current().push_scissors(scissors->world_rect(transform->worldMatrix));
        }
    }

    bool programChanged = false;
    auto* display = e.tryGet<Display2D>();
    if (display) {
        if (display->program) {
            programChanged = true;
            draw2d::current().saveProgram().setProgram(display->program.get());
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
                draw2d::current().save_transform();
                draw2d::current().concat(childTransform->matrix);
                draw2d::current().concat(childTransform->color);
            }
            if (draw2d::current().color.scale.a > 0) {
                drawStack(it);
            }
            if (childTransform) {
                draw2d::current().restore_transform();
            }
        }
        it = child.sibling_next;
    }

    if (bounds && bounds->scissors) {
        draw2d::current().popClipRect();
    }
    if (programChanged) {
        draw2d::current().restoreProgram();
    }
}
}