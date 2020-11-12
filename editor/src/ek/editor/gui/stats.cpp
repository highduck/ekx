#include "gui.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/util/locator.hpp>
#include <ek/editor/imgui/imgui.hpp>

namespace ek {

void guiStatsWindow(bool* p_open) {
    if (ImGui::Begin("Stats", p_open)) {
        auto& app = resolve<basic_application>();
        draw2d::flush_batcher();
        ImGui::Text("%0.2lf ms | dc: %u | tri: %u",
                    app.frame_timer.delta_time() * 1000.0,
                    draw2d::get_stat_draw_calls(),
                    draw2d::get_stat_triangles()
        );
        const auto& pool = ecxx::get_world().pool();
        ImGui::Text("%u entities | %u free", pool.size(), pool.available_for_recycling());
    }
    ImGui::End();
}

}