#pragma once

#include <array>
#include <string>

namespace ek {

std::string get_device_lang();

std::array<int, 4> get_screen_insets();

void vibrate(int duration_millis);

std::string getDeviceFontPath(const char* fontName);

}
