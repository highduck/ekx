#pragma once

#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/util/ServiceLocator.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include "Widgets.hpp"

namespace ek {

static float getterProfilerTrackValue(void* data, int idx) {
    auto* track = static_cast<Profiler::Track*>(data);
    return track->history.at(idx);
}

void StatsWindow::onDraw() {
    auto& app = Locator::ref<basic_application>();
    auto stats = draw2d::state.stats;
    const float drawableArea = ek_app.viewport.width * ek_app.viewport.height;
    ImGui::Text("%ld µs | dc: %u | tri: %u | fill: %d%%",
                (long)(app.frameTimer.deltaTime * 1000000.0f),
                stats.drawCalls,
                stats.triangles,
                (int)(100.0f * stats.fillArea / drawableArea)
    );
    auto entitiesCount = ecs::the_world.size;
    auto entitiesAvailable = ecs::ENTITIES_MAX_COUNT - entitiesCount;
    ImGui::Text("%u entities | %u free", entitiesCount - 1, entitiesAvailable);

    auto hitTarget = Locator::ref<InteractionSystem>().getHitTarget();
    if (hitTarget) {
        ImGui::Text("Hit Target: %u %s", hitTarget.index, getDebugNodePath(hitTarget).c_str());
    }

    auto* baseApp = Locator::get<basic_application>();
    if(baseApp) {
        auto& profiler = baseApp->profiler;
        for(auto& track : profiler.tracks) {
            ImGui::PushID(&track);
            ImGui::PlotLines("", &getterProfilerTrackValue, &track, track.history.size());
            ImGui::SameLine();
            ImGui::Text(track.titleFormat, track.name, (int)track.value);
            ImGui::PopID();
        }
    }
}

}