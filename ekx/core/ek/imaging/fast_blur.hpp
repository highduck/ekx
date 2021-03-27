#pragma once

#include <cstdint>

namespace ek {

void fastBlurA8(uint8_t* data, int width, int height, int stride, float radius, int iterations, int strength);

}

