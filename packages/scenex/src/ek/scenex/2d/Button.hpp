#pragma once

#include <ek/util/Signal.hpp>
#include <ek/math.h>
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
};

struct Button {
    ButtonSkin* skin{nullptr};

    bool back_button = false;
    Signal<> clicked{};

    bool initialized = false;
    float timeOver = 0.0f;
    float timePush = 0.0f;
    float timePost = 0.0f;

    vec2_t baseSkew = {};
    vec2_t baseScale = vec2(1,1);
    color2_t baseColor = color2_identity();

    TimeLayer time = TimeLayer::UI;

    static void updateAll();
};

}


