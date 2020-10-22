#pragma once

#include <vector>
#include <cstdint>

namespace ek {

class image_t;

image_t* decode_image_data(const std::vector<uint8_t>& data);

}