#include "gui_window_stats.h"

#include <scenex/ek/basic_application.h>
#include <baseapp/base_app.hpp>
#include <draw2d/drawer.hpp>
#include <ek/util/locator.hpp>
#include <ek/editor/imgui/imgui.hpp>

namespace scenex {

void gui_draw_stats() {
    auto& app = ek::resolve<basic_application>();
    auto& drawer = ek::resolve<ek::drawer_t>();

    drawer.batcher.flush();
    ImGui::Begin("stats");
    //ImGui::LabelText("fps", "%f", app.stats_.fps());
    ImGui::LabelText("dc", "%u", drawer.batcher.stats.draw_calls);
    ImGui::LabelText("tris", "%u", drawer.batcher.stats.triangles);

    ImGui::End();
}

}