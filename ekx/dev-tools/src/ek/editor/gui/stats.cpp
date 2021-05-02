#include "gui.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/util/locator.hpp>
#include <ek/editor/imgui/imgui.hpp>

namespace ek {

std::string getDebugNodePath(ecs::EntityApi e) {
    std::string result{};
    while(e) {
        result = e.get_or_default<NodeName>().name + "/" + result;
        e = e.get<Node>().parent;
    }
    return result;
}

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
        auto entitiesCount = ecs::the_world.size;
        auto entitiesAvailable = ecs::ENTITIES_MAX_COUNT - entitiesCount;
        ImGui::Text("%u entities | %u free", entitiesCount - 1, entitiesAvailable);
        ImGui::Text("Batching buffer objects: %0.2f MB", (draw2d::state.getUsedMemory() / 1000000.0f));

        auto hitTarget = resolve<InteractionSystem>().getHitTarget();
        if(hitTarget) {
            ImGui::Text("Hit Target: %u %s", hitTarget.index, getDebugNodePath(hitTarget).c_str());
        }
    }
    ImGui::End();
}

}