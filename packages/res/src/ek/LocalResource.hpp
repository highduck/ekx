#pragma once

#include <cstdint>
#include <vector>
#include <functional>

namespace ek {

using array_buffer = std::vector<uint8_t>;
using get_content_callback_func = std::function<void(array_buffer)>;

void get_resource_content_async(const char* path, const get_content_callback_func& callback);

}
