#pragma once

#include <string>
#include <ek/math/easing.hpp>
#include <ek/util/signals.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>

namespace ek {

struct button_skin {
    std::string sfx_over = "sfx/btn_over";
    std::string sfx_out = "sfx/btn_out";
    std::string sfx_down = "sfx/btn_down";
    std::string sfx_click = "sfx/btn_click";

    float over_speed_forward = 8.0f;
    float over_speed_backward = 8.0f;
    float push_speed_forward = 8.0f;
    float push_speed_backward = 8.0f;

    //const basic_ease_t& over_ease{easing::P3_OUT};
    //const basic_ease_t& push_ease{easing::P3_OUT};
};

struct button_t {
    button_skin* skin{nullptr};

    bool back_button = false;
    signal_t<> clicked;

    bool initialized = false;
    float over_time = 0.0f;
    float push_time = 0.0f;
    float post_time = 0.0f;

    float2 baseSkew{};
    float2 baseScale = float2::one;
    ColorMod32 baseColor{};
};

}


