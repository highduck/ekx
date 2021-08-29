#pragma once

#include <string>
#include <ek/math/easing.hpp>
#include <ek/util/Signal.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>
#include "MovieClip.hpp"

namespace ek {

struct ButtonSkin {
    const char* sfxOver = "sfx/btn_over";
    const char* sfxDown = "sfx/btn_down";
    const char* sfxClick = "sfx/btn_click";
    const char* sfxOut = "sfx/btn_out";
    const char* sfxCancel = "sfx/btn_cancel";

    float overSpeedForward = 8.0f;
    float overSpeedBackward = 8.0f;
    float pushSpeedForward = 8.0f;
    float pushSpeedBackward = 8.0f;

    //const basic_ease_t& over_ease{easing::P3_OUT};
    //const basic_ease_t& push_ease{easing::P3_OUT};
};

struct Button {
    ButtonSkin* skin{nullptr};

    bool back_button = false;
    Signal<> clicked{};

    bool initialized = false;
    float timeOver = 0.0f;
    float timePush = 0.0f;
    float timePost = 0.0f;

    float2 baseSkew{};
    float2 baseScale = float2::one;
    ColorMod32 baseColor{};

    TimeLayer time = TimeLayer::UI;

    static void updateAll();
};

EK_DECLARE_TYPE(Button);

}


