#include "profiler.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <ek/canvas.h>
#include <ek/app.h>

// TODO: EK_DEV_BUILD
#ifndef ENABLE_PROFILER

#if !defined(NDEBUG) || defined(EK_DEV_TOOLS)
#define ENABLE_PROFILER
#endif

#endif

namespace ek {

void FpsMeter::update(float dt) {
    if (measurementsPerSeconds > 0.0f) {
        // estimate average FPS for some period
        counter_ += 1.0f;
        accum_ += dt;
        const auto period = 1.0f / measurementsPerSeconds;
        if (accum_ >= period) {
            avgFPS_ = counter_ * measurementsPerSeconds;
            accum_ -= period;
            counter_ = 0.0f;
        }
    } else {
        avgFPS_ = dt > 0.0f ? (1.0f / dt) : 0.0f;
    }
}

using namespace std;

void drawText(const Profiler::Track& track, TextFormat textFormat) {
    textFormat.leading = 1;
    auto& textEngine = get_text_engine()->engine;
    textEngine.format = textFormat;
    textEngine.position = {2.0f, 2.0f + textFormat.size};
    textEngine.drawFormat(track.titleFormat, track.name, (int)track.value);
}

void drawGraph(const Profiler::Track& track) {
    float width = (float) track.history.size();

    const int samples = track.history.capacity();
    float x = 0.0f;
    float x0 = 0.0f;
    float prev = track.calculateY(track.history.at(0));
    canvas_set_empty_image();
    canvas_quad_color(0, 0, width, track.height, ARGB(0x77000000));
    for (int i = 0; i < samples; ++i) {
        float val = track.history.at(i);
        float curr = track.calculateY(val);
        if (curr != prev || i + 1 == samples) {
            if (x > x0) {
                canvas_quad_color(x0, track.height - prev, x - x0, 1, track.calculateColor(prev));
            }
            if (curr != prev) {
                canvas_quad_color(x, track.height - curr, 1, curr - prev, track.calculateColor(curr));
            }
            x0 = x + 1;
        }
        prev = curr;
        x += 1;
    }
}

void draw(Profiler& profiler, const GameDisplayInfo& displayInfo) {
#ifdef ENABLE_PROFILER
    if (!profiler.enabled) {
        return;
    }
    const auto scale = displayInfo.dpiScale;
    const auto x = displayInfo.insets.x;
    const auto y = displayInfo.insets.y;
    canvas.matrix[0].rot = {scale, 0, 0, scale};
    canvas.matrix[0].pos = displayInfo.insets.xy;
    canvas_set_empty_image();

    canvas_save_matrix();
    for (auto& track : profiler.tracks) {
        drawGraph(track);
        canvas_translate({0, 35});
    }
    canvas_restore_matrix();

    canvas_save_matrix();
    for (auto& track : profiler.tracks) {
        drawText(track, profiler.textFormat);
        canvas_translate({0, 35});
    }
    canvas_restore_matrix();
#endif
}

void Profiler::update(float dt) {
#ifdef ENABLE_PROFILER
    fpsMeter.update(dt);
    tracks[Track_FPS].history.write(dt * 1000.0f);
    tracks[Track_FPS].value = fpsMeter.getAverageFPS();

    for(uint32_t i = DefaultTracks_Count; i < tracks.size(); ++i) {
        auto& track = tracks[i];
        track.history.write(track.accum);
        track.value = track.accum;
        track.accum = 0.0f;
    }
#endif
}

Profiler::Profiler() {
#ifdef ENABLE_PROFILER
    tracks.resize(DefaultTracks_Count);
    {
        auto& track = tracks[Track_FPS];
        track.name = "FPS";
        track.titleFormat = "%s: %d";
        track.thMin = 1000.0f / 60.0f;
        track.thMax = 1000.0f / 30.0f;
        track.min = 16.0f;
        track.max = 100.0f;
    }
    {
        auto& track = tracks[Track_Triangles];
        track.name = "TRIS";
        track.titleFormat = "%s: %d";
        track.thMin = 1000;
        track.thMax = 2000;
        track.min = 0;
        track.max = 3000;
    }

    {
        auto& track = tracks[Track_DrawCalls];
        track.name = "DC";
        track.titleFormat = "%s: %d";
        track.thMin = 20;
        track.thMax = 50;
        track.min = 0;
        track.max = 200;
    }

    {
        auto& track = tracks[Track_FillRate];
        track.name = "FR";
        track.titleFormat = "%s: %d";
        track.thMin = 1;
        track.thMax = 3;
        track.min = 0;
        track.max = 5;
    }
#endif
}

Profiler::~Profiler() = default;

void Profiler::addTime(const char* name, float time) {
    (void) name;
    (void) time;
#ifdef ENABLE_PROFILER
    {
        Track* track = nullptr;
        for (uint32_t i = DefaultTracks_Count; i < tracks.size(); ++i) {
            if (tracks[i].name == name) {
                track = &tracks[i];
                break;
            }
        }
        if (!track) {
            Track t{};
            t.name = name;
            t.titleFormat = "%s: %d";
            t.thMin = 4;
            t.thMax = 8;
            t.min = 0;
            t.max = 16;
            track = &tracks.emplace_back(t);
        }
        if (track) {
            track->accum += time;
        }
    }
#endif
}

void Profiler::beginRender(float drawableArea_) {
#ifdef ENABLE_PROFILER
    auto stats = canvas.stats;
    drawableArea = drawableArea_;
    FR = stats.fill_area;
    DC = stats.draw_calls;
    TR = stats.triangles;
#endif
}

void Profiler::endRender() {
#ifdef ENABLE_PROFILER
    const auto stats = canvas.stats;
    DC = stats.draw_calls - DC;
    FR = stats.fill_area - FR;
    TR = stats.triangles - TR;
    tracks[Track_DrawCalls].history.write((float)DC);
    tracks[Track_DrawCalls].value = (float)DC;
    tracks[Track_Triangles].history.write((float)TR);
    tracks[Track_Triangles].value = (float)TR;

    {
        const auto fillRate = FR / drawableArea;
        tracks[Track_FillRate].history.write(fillRate);
        tracks[Track_FillRate].value = fillRate;
    }
#endif
}

}