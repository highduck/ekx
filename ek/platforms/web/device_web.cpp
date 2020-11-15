#include <ek/app/device.hpp>

namespace ek {

float4 get_screen_insets() {
    return float4{};
}

//void vibrate(int duration_millis);

std::string getDeviceFontPath(const char* fontName) {
    return "baseline.ttf";
}

}