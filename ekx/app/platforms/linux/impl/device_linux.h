#pragma once

#include <ek/app/app.hpp>

namespace ek::app {

int openURL(const char*) {
    return -1;
}

int vibrate(int millis) {
    (void)millis;
    // unsupported
    return -1;
}

const float* getScreenInsets() {
    static float pads[4];
    pads[0] = 0.0f;
    pads[1] = 0.0f;
    pads[2] = 0.0f;
    pads[3] = 0.0f;
    return pads;
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