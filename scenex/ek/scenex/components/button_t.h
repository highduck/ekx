#pragma once

#include <string>
#include <ek/math/easing.hpp>
#include <ek/util/signals.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>

namespace ek {

class button_skin {
public:
    std::string sfxOver = "over";
    std::string sfxOut = "out";
    std::string sfxDown = "down";
    std::string sfxClick = "click";

    float overSpeedForward = 8.0f;
    float overSpeedBackward = 8.0f;
    float pressSpeedForward = 8.0f;
    float pressSpeedBackward = 8.0f;

    const basic_ease_t& overEase{easing::P3_OUT};
    const basic_ease_t& pushEase{easing::P3_OUT};
};

struct button_t {
    button_skin* skin{nullptr};

    bool back_button = false;
    signal_t<> clicked;

    bool initialized = false;
    float over_time = 0.0f;
    float push_time = 0.0f;
    float post_time = 0.0f;

    float2 base_skew{};
    float2 base_scale = float2::one;
    argb32_t base_color_multiplier = argb32_t::one;
    argb32_t base_color_offset = argb32_t::zero;
};

}


