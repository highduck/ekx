#include "gui_window_stats.h"

#include <ek/scenex/ek/basic_application.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/util/locator.hpp>
#include <ek/editor/imgui/imgui.hpp>

namespace ek {

void gui_draw_stats() {
    auto& app = resolve<basic_application>();
    auto& drawer = ek::resolve<ek::drawer_t>();

    drawer.batcher.flush();
    ImGui::Begin("stats");
    //ImGui::LabelText("fps", "%f", app.stats_.fps());
    ImGui::LabelText("dc", "%u", drawer.batcher.stats.draw_calls);
    ImGui::LabelText("tris", "%u", drawer.batcher.stats.triangles);

    ImGui::End();
}

}