#pragma once

#include <ecx/ecx.hpp>
#include <ek/math.h>
#include <ekx/app/time_layers.h>

namespace ek {

//struct Shake {
//    float time = 0.0f;
//    float time_total = 1.0f;
//    float strength = 16.0f;
//    TimeLayer timer;
//
//    static void updateAll();
//    static void add(ecs::Entity e, float time = 2.0f, float strength = 16.0f);
//};

struct Shaker {
    TimeLayer timer;
    float state = 0.0f;
    float maxRotation = 0.25f;
    vec2_t maxOffset = vec2(8.0f, 8.0f);
    vec2_t maxScale = vec2(0.25f, 0.25f);

    void start(float v = 1.0f) {
        state = MAX(v, state);
    }

    static void updateAll();
};



}


