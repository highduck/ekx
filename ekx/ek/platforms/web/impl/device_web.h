#pragma once

#include <ek/app/device.hpp>

namespace ek {

extern "C" {
extern void web_vibrate(int cur);
extern char* web_get_lang();
}

void getScreenInsets(float padding[4]) {
    padding[0] = 0.0f;
    padding[1] = 0.0f;
    padding[2] = 0.0f;
    padding[3] = 0.0f;
}

void vibrate(int duration_millis) {
    web_vibrate(duration_millis);
}

std::string get_device_lang() {
    char* str = web_get_lang();
    if (str) {
        std::string result{str};
        free(str);
        return result;
    }
    return "en";
}

std::string getDeviceFontPath(const char* fontName) {
    (void)fontName;
    return "baseline.ttf";
}

}