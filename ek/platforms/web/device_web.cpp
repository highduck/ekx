#include <ek/app/device.hpp>

namespace ek {

std::array<int, 4> get_screen_insets() {
    return {0, 0, 0, 0};
}

//void vibrate(int duration_millis);

std::string getDeviceFontPath(const char* fontName) {
    return "baseline.ttf";
}

}