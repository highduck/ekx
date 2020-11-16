#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>
#include <string>

namespace ek {

struct bubble_text_t {
    float delay = 0.0f;
    float time = 0.0f;
    float2 start{};
    float2 offset{};
};

void update_bubble_text(float dt);
ecs::entity create_bubble_text(const float2& pos, const std::string& text, float delay = 0.0f);

}


