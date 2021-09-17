#pragma once

#include <ek/app/app.hpp>

int main(int argc, char* argv[]) {
    ::ek::app::g_app.argc = argc;
    ::ek::app::g_app.argv = argv;
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