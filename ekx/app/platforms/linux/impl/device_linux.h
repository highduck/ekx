#pragma once

#include <ek/app/app.hpp>

namespace ek::app {

const float* getScreenInsets() {
    static float pads[4];
    pads[0] = 0.0f;
    pads[1] = 0.0f;
    pads[2] = 0.0f;
    pads[3] = 0.0f;
    return pads;
}

void vibrate(int millis) {
    // TODO:
    (void)millis;
}

const char* getPreferredLang() {
    // TODO:
    return "en";
}

const char* getSystemFontPath(const char* fontName) {
    // TODO:
    (void) fontName;
    return nullptr;
}

}