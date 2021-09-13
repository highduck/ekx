#pragma once

#include <ek/app/app.hpp>
#include <ek/Arguments.hpp>

int main(int argc, char* argv[]) {
    ::ek::Arguments::current = {argc, argv};
    ::ek::app::main();
    return 0;
}

namespace ek::app {

const float* getScreenInsets() {
    return nullptr;
}

void vibrate(int millis) {
    // TODO:
    (void)millis;
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