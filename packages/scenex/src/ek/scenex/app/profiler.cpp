#include "profiler.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <ek/draw2d/drawer.hpp>
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
    auto& textEngine = gTextEngine.get().engine;
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
    draw2d::state.set_empty_image();
    draw2d::quad(0, 0, width, track.height, 0x77000000_argb);
    for (int i = 0; i < samples; ++i) {
        float val = track.history.at(i);
        float curr = track.calculateY(val);
        if (curr != prev || i + 1 == samples) {
            if (x > x0) {
                draw2d::quad(x0, track.height - prev, x - x0, 1, track.calculateColor(prev));
            }
            if (curr != prev) {
                draw2d::quad(x, track.height - curr, 1, curr - prev, track.calculateColor(curr));
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
    draw2d::state.matrix = Matrix3x2f{scale, 0, 0, scale, x, y};
    draw2d::state.set_empty_image();

    draw2d::state.save_matrix();
    for (auto& track : profiler.tracks) {
        drawGraph(track);
        draw2d::state.translate(0, 35);
    }
    draw2d::state.restore_matrix();

    draw2d::state.save_matrix();
    for (auto& track : profiler.tracks) {
        drawText(track, profiler.textFormat);
        draw2d::state.translate(0, 35);
    }
    draw2d::state.restore_matrix();
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
    auto stats = draw2d::getDrawStats();
    drawableArea = drawableArea_;
    FR = stats.fillArea;
    DC = stats.drawCalls;
    TR = stats.triangles;
#endif
}

void Profiler::endRender() {
#ifdef ENABLE_PROFILER
    const auto stats = draw2d::getDrawStats();
    DC = stats.drawCalls - DC;
    FR = stats.fillArea - FR;
    TR = stats.triangles - TR;
    tracks[Track_DrawCalls].history.write(DC);
    tracks[Track_DrawCalls].value = DC;
    tracks[Track_Triangles].history.write(TR);
    tracks[Track_Triangles].value = TR;

    {
        const auto fillRate = FR / drawableArea;
        tracks[Track_FillRate].history.write(fillRate);
        tracks[Track_FillRate].value = fillRate;
    }
#endif
}

}