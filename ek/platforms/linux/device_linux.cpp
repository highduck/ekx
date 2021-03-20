#include <ek/app/device.hpp>

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
