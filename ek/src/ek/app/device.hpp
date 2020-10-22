#pragma once

#include <array>
#include <string>

namespace ek {

[[maybe_unused]] std::string get_device_lang();

std::array<int, 4> get_screen_insets();

void vibrate(int duration_millis);

}
