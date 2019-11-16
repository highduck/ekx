#pragma once

#include <ek/array_buffer.hpp>

namespace ek {

class image_t;

image_t* decode_image_data(const array_buffer& buffer);

}
