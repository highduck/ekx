#include "mouse_follow_script.hpp"

#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/util/locator.hpp>

namespace ek {

void mouse_follow_script::update(float dt) {
    Script::update(dt);

    auto parent = entity_.get<Node>().parent;
    if (parent) {
        auto& im = resolve<InteractionSystem>();
        const auto pos = Transform2D::globalToLocal(parent, im.pointerScreenPosition_);
        get<Transform2D>().position = pos;
    }
}

void mouse_follow_script::gui_gizmo() {
    draw2d::fill_circle(circle_f{100.0f}, 0x00FFFFFF_argb, 0x77FFFFFF_argb, 10);
}

void mouse_follow_script::gui_inspector() {
//#if defined(DEV_MODE)
//    if (ImGui::CollapsingHeader("Mouse Follow")) {}
//#endif
}

}