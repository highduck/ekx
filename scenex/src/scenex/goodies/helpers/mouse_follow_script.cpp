#include "mouse_follow_script.h"

#include <draw2d/drawer.hpp>
#include <ek/locator.hpp>
#include <scenex/utility/scene_management.h>
#include <scenex/interactive_manager.h>

using namespace ek;

namespace scenex {

void mouse_follow_script::update(float dt) {
    script::update(dt);

    auto& im = resolve<interactive_manager>();
    const auto screen_space_position = im.pointer_global_space;
    const auto parent_space_position = global_to_parent(entity_, screen_space_position);
    get<transform_2d>().matrix.position(parent_space_position);
}

void mouse_follow_script::gui_gizmo() {
    resolve<drawer_t>().fill_circle(circle_f{100.0f}, 0x00FFFFFF_argb, 0x77FFFFFF_argb, 10);
}

void mouse_follow_script::gui_inspector() {
//#if defined(DEV_MODE)
//    if (ImGui::CollapsingHeader("Mouse Follow")) {}
//#endif
}

}