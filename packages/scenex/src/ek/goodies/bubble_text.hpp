#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math.h>
#include <ek/ds/String.hpp>

namespace ek {

struct BubbleText {
    float delay = 0.0f;
    float time = 0.0f;
    vec2_t start = {};
    vec2_t offset = {};

    static void updateAll();

    static ecs::EntityApi create(string_hash_t fontName, const String& text, vec2_t pos, float delay = 0.0f);
};

}
