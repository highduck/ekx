#include "mouse_follow_script.hpp"

#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/canvas.h>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/util/ServiceLocator.hpp>

namespace ek {

void mouse_follow_script::update(float dt) {
    Script::update(dt);

    auto parent = entity_.get<Node>().parent;
    if (parent) {
        auto& im = Locator::ref<InteractionSystem>();
        const auto& camera = Camera2D::Main.get<Camera2D>();
        const auto cameraPointer = vec2_transform(im.pointerScreenPosition_, camera.screenToWorldMatrix);
        const auto pos = Transform2D::globalToLocal(parent, cameraPointer);
        get<Transform2D>().setPosition(pos);
    }
}

void mouse_follow_script::gui_gizmo() {
    canvas_fill_circle({0.0f, 0.0f, 100.0f}, 0x00FFFFFF_argb, 0x77FFFFFF_argb, 10);
}

void mouse_follow_script::gui_inspector() {
//#if defined(DEV_MODE)
//    if (ImGui::CollapsingHeader("Mouse Follow")) {}
//#endif
}

}