#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>
#include <string>

namespace ek {

struct BubbleText {
    float delay = 0.0f;
    float time = 0.0f;
    float2 start{};
    float2 offset{};

    static void updateAll();

    static ecs::entity create(const char* fontName, const std::string& text, const float2& pos, float delay = 0.0f);
};


}


