#pragma once

#include <string>
#include <ek/array_buffer.hpp>

namespace ek {

array_buffer base64_decode(const std::string& str);
std::string base64_decode_str(const std::string& str);

std::string base64_encode(const array_buffer& data);
std::string base64_encode(const std::string& str);

}

