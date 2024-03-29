#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/timers.hpp>
#include <ek/math/vec.hpp>

namespace ek {

struct Shake {
    float time = 0.0f;
    float time_total = 1.0f;
    float strength = 16.0f;
    TimeLayer timer;

    static void updateAll();
    static void add(ecs::EntityApi e, float time = 2.0f, float strength = 16.0f);
};

struct Shaker {
    TimeLayer timer;
    float state = 0.0f;
    float maxRotation = 0.25f;
    float2 maxOffset{8.0f, 8.0f};
    float2 maxScale{0.25f, 0.25f};

    void start(float v = 1.0f) {
        state = std::max(v, state);
    }

    static void updateAll();
};

}


