#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace ek::base64 {

std::string encode(const std::vector<uint8_t>& data);

std::vector<uint8_t> decode(const std::string& str);

size_t encoded_size(size_t bytes_count);

size_t encode(char* out_str, const uint8_t* bytes, size_t bytes_count);

size_t decoded_size(size_t string_length);

size_t decode(uint8_t* out_buffer, const char* str, size_t chars_to_decode);

}

