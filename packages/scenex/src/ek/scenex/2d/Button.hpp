#pragma once

#include <ek/math/Easings.hpp>
#include <ek/util/Signal.hpp>
#include <ek/math/Vec.hpp>
#include <ek/math/Color32.hpp>
#include "MovieClip.hpp"
#include "../base/TimeLayer.hpp"

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

    Vec2f baseSkew{};
    Vec2f baseScale = Vec2f::one;
    ColorMod32 baseColor{};

    TimeLayer time = TimeLayer::UI;

    static void updateAll();
};

ECX_TYPE(9, Button);

}


