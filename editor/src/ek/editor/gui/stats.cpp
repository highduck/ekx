#include "gui.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/util/locator.hpp>
#include <ek/editor/imgui/imgui.hpp>

namespace ek {

void guiStatsWindow(bool* p_open) {
    if (ImGui::Begin("Stats", p_open)) {
        auto& app = resolve<basic_application>();
        auto stats = draw2d::state.stats;
        auto drawableSize = ek::app::g_app.drawable_size;
        ImGui::Text("%0.2lf ms | dc: %u | tri: %u | fill: %0.2f",
                    app.frame_timer.delta_time() * 1000.0,
                    stats.drawCalls,
                    stats.triangles,
                    stats.fillArea / (drawableSize.x * drawableSize.y)
        );
        auto entitiesCount = ecs::the_world.count;
        auto entitiesAvailable = ecs::ENTITIES_MAX_COUNT - entitiesCount;
        ImGui::Text("%u entities | %u free", entitiesCount - 1, entitiesAvailable);
        ImGui::Text("Batching buffer objects: %0.2f MB", (draw2d::state.getUsedMemory() / 1000000.0f));
    }
    ImGui::End();
}

}