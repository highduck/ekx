#pragma once

#include <ek/util/Signal.hpp>
#include <ek/math.h>
#include "MovieClip.hpp"
#include <ekx/app/time_layers.h>

#define BUTTON_EVENT_CLICK H("button_click")

namespace ek {

struct ButtonSkin {
    string_hash_t sfxOver = H("sfx/btn_over");
    string_hash_t sfxDown = H("sfx/btn_down");
    string_hash_t sfxClick = H("sfx/btn_click");
    string_hash_t sfxOut = H("sfx/btn_out");
    string_hash_t sfxCancel = H("sfx/btn_cancel");

    float overSpeedForward = 8.0f;
    float overSpeedBackward = 8.0f;
    float pushSpeedForward = 8.0f;
    float pushSpeedBackward = 8.0f;
};

struct Button {
    ButtonSkin* skin = nullptr;

    float timeOver = 0.0f;
    float timePush = 0.0f;
    float timePost = 0.0f;

    vec2_t baseSkew = {};
    vec2_t baseScale = vec2(1,1);
    color2_t baseColor = color2_identity();

    TimeLayer time = TIME_LAYER_UI;

    bool initialized = false;
//    bool back_button = false;
    bool pushed = false;

    static void updateAll();
};

}


