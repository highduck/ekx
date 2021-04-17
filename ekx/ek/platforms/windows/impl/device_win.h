#pragma once

#include <ek/app/device.hpp>
#include <ek/app/app.hpp>
#include <ek/Arguments.hpp>

int main(int argc, char* argv[]) {
    ::ek::Arguments::current = {argc, argv};
    ::ek::main();
    return 0;
}

namespace ek {

void getScreenInsets(float padding[4]) {
    padding[0] = 0.0f;
    padding[1] = 0.0f;
    padding[2] = 0.0f;
    padding[3] = 0.0f;
}

void vibrate(int duration_millis) {
}

std::string get_device_lang() {
    return "en";
}

std::string getDeviceFontPath(const char* fontName) {
    return "baseline.ttf";
}

}