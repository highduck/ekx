#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/Vec.hpp>
#include <ek/ds/String.hpp>

namespace ek {

struct BubbleText {
    float delay = 0.0f;
    float time = 0.0f;
    Vec2f start{};
    Vec2f offset{};

    static void updateAll();

    static ecs::EntityApi create(const char* fontName, const String& text, const Vec2f& pos, float delay = 0.0f);
};

ECX_TYPE(20, BubbleText);

}


