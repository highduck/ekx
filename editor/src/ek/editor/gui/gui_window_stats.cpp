#include "gui_window_stats.h"

#include <ek/scenex/app/basic_application.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/util/locator.hpp>
#include <ek/editor/imgui/imgui.hpp>

namespace ek {

void gui_draw_stats() {
    auto& app = resolve<basic_application>();

    draw2d::flush_batcher();

    ImGui::Begin("stats");
    //ImGui::LabelText("fps", "%f", app.stats_.fps());
    ImGui::LabelText("dc", "%u", draw2d::get_stat_draw_calls());
    ImGui::LabelText("tris", "%u", draw2d::get_stat_triangles());

    ImGui::End();
}

}