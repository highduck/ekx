#pragma once

#include <array>
#include <string>
#include <ek/math/vec.hpp>

namespace ek {

std::string get_device_lang();

// TODO: return float4 ?
float4 get_screen_insets();

void vibrate(int duration_millis);

std::string getDeviceFontPath(const char* fontName);

}
