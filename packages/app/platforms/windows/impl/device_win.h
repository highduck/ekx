#pragma once

#include <ek/app/app.hpp>

namespace ek::app {

int openURL(const char*) {
    return -1;
}

const float* getScreenInsets() {
    return nullptr;
}

const char* getPreferredLang() {
    // TODO:
    return nullptr;
}

const char* getSystemFontPath(const char* fontName) {
    // TODO:
    (void) fontName;
    return nullptr;
}

}