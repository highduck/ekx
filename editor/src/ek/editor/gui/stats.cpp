#include "gui.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/util/locator.hpp>
#include <ek/editor/imgui/imgui.hpp>

namespace ek {

void guiStatsWindow(bool *p_open) {
    if (ImGui::Begin("Stats", p_open)) {
        auto& app = resolve<basic_application>();
        draw2d::flush_batcher();
        ImGui::Text("%0.2lf ms | dc: %u | tri: %u",
                    app.frame_timer.delta_time() * 1000.0,
                    draw2d::get_stat_draw_calls(),
                    draw2d::get_stat_triangles()
        );
        const auto& entities = ecs::world::the.entities;
        ImGui::Text("%u entities | %u free", entities.size(), entities.available_for_recycling());
        ImGui::Text("Batching buffer objects: %0.2f MB", (draw2d::getBatchingUsedMemory() / 1000000.0f));
    }
    ImGui::End();
}

}