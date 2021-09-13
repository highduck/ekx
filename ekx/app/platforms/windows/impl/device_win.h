#pragma once

#include <ek/app/app.hpp>
#include <ek/Arguments.hpp>

int main(int argc, char* argv[]) {
    ::ek::Arguments::current = {argc, argv};
    ::ek::app::main();
    return 0;
}

namespace ek::app {

int openURL(const char*) {
    return -1;
}

const float* getScreenInsets() {
    return nullptr;
}

int vibrate(int millis) {
    (void)millis;
    return -1;
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