#include <ek/app/device.hpp>

namespace ek {

extern "C" {
extern void web_vibrate(int cur);
extern char* web_get_lang();
}

float4 get_screen_insets() {
    return float4{};
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
    return "baseline.ttf";
}

}