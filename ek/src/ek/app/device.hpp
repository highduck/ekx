#pragma once

#include <array>
#include <string>
#include <ek/math/vec.hpp>

namespace ek {

std::string get_device_lang();

// safe area margins left-top-right-bottom
void getScreenInsets(float padding[4]);

void vibrate(int duration_millis);

std::string getDeviceFontPath(const char* fontName);

}
