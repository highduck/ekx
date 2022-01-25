#pragma once

#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/canvas.h>

#include <ek/editor/imgui/imgui.hpp>
#include "Widgets.hpp"

namespace ek {

static float getterProfilerTrackValue(void* data, int idx) {
    struct profiler_track* track = (struct profiler_track*) data;
    return track->hist[(track->hist_i + idx) % PROFILE_TRACK_CAPACITY];
}

void StatsWindow::onDraw() {
    auto* base_app = g_game_app;
    if (!base_app) {
        return;
    }

    auto stats = canvas.stats;
    const float drawableArea = ek_app.viewport.width * ek_app.viewport.height;
    ImGui::Text("%ld µs | dc: %u | tri: %u | fill: %d%%",
                (long) (base_app->frame_timer.dt * 1000000.0f),
                stats.draw_calls,
                stats.triangles,
                (int) (100.0f * stats.fill_area / drawableArea)
    );
    auto entitiesCount = ecs::the_world.size;
    auto entitiesAvailable = ecs::ENTITIES_MAX_COUNT - entitiesCount;
    ImGui::Text("%u entities | %u free", entitiesCount - 1, entitiesAvailable);

    auto hitTarget = g_interaction_system->getHitTarget();
    if (hitTarget) {
        ImGui::Text("Hit Target: %u %s", hitTarget.index, getDebugNodePath(hitTarget).c_str());
    }

    for (uint32_t i = 0; i < PROFILE_TRACKS_MAX_COUNT; ++i) {
        auto* track = &s_profile_metrics.tracks[i];
        ImGui::PushID(track);
        ImGui::PlotLines("", &getterProfilerTrackValue, track, PROFILE_TRACK_CAPACITY);
        ImGui::SameLine();
        ImGui::Text(track->titleFormat, track->name, (int) track->value);
        ImGui::PopID();
    }
}

}