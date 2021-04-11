#pragma once

#include <cstddef>

namespace ek {

class image_t;

image_t* decode_image_data(const void* data, size_t size, bool premultiplyAlpha = true);

}