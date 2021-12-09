#pragma once

#include <ek/ds/StaticHistoryBuffer.hpp>
#include <ek/ds/Array.hpp>
#include <ek/math/Color32.hpp>
#include <ek/timers.hpp>
#include <ek/scenex/text/TextEngine.hpp>
#include "GameDisplay.hpp"

namespace ek {

class FpsMeter final {
public:
    void update(float dt);

    [[nodiscard]]
    float getAverageFPS() const {
        return avgFPS_;
    }

private:
    // if less or equal 0 - calculate FPS for every frame
    const float measurementsPerSeconds = 2.0f;
    float avgFPS_ = 0.0f;
    float counter_ = 0.0f;
    float accum_ = 0.0f;
};

class Profiler {
public:
    struct Track {
        const char* name = "";
        const char* titleFormat = "%s: %d";
        float thMin = 1.0f / 60.0f;
        float thMax = 1.0f / 30.0f;
        float accum{};
        float value{};
        StaticHistoryBuffer<float, 60> history{};

        float height = 30.0f;
        float min = 0.0f;
        float max = 33.3f;

        [[nodiscard]] float calculateY(float val) const {
            return roundf(height * (val - min) / (max - min));
        }

        [[nodiscard]] argb32_t calculateColor(float val) const {
            return lerp(0x00FF00_rgb, 0xFF0000_rgb,
                        Math::clamp((val - thMin) / (thMax - thMin), 0.0f, 1.0f));
        }
    };

    enum {
        Track_FPS = 0,
        Track_Triangles = 1,
        Track_DrawCalls = 2,
        Track_FillRate = 3,
        DefaultTracks_Count = 4
    };
    TextFormat textFormat{"Cousine-Regular", 10};
    FpsMeter fpsMeter{};
    Array<Track> tracks{};
    bool enabled = true;

    float drawableArea = 1.0f;
    float FR;
    uint32_t DC;
    uint32_t TR;

    Profiler();

    ~Profiler();

    void update(float dt);

    void addTime(const char* name, float time);

    void beginRender(float drawableArea_);
    void endRender();
};

void draw(Profiler& profiler, const GameDisplayInfo& displayInfo);

}

