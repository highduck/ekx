#include "mouse_follow_script.hpp"

#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/interactive_manager.hpp>
#include <ek/util/locator.hpp>

namespace ek {

void mouse_follow_script::update(float dt) {
    script_cpp::update(dt);

    auto& im = resolve<interactive_manager>();
    const auto screen_space_position = im.pointer_global_space;
    const auto parent_space_position = global_to_parent(entity_, screen_space_position);
    get<transform_2d>().matrix.position(parent_space_position);
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