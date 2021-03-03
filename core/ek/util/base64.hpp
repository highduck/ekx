#pragma once

#include <cstdint>

namespace ek::base64 {

uint32_t getEncodedMaxSize(uint32_t bytes_count);

uint32_t encode(char* out_str, const uint8_t* bytes, uint32_t bytes_count);

uint32_t getDecodedMaxSize(uint32_t string_length);

uint32_t decode(uint8_t* out_buffer, const char* str, uint32_t chars_to_decode);

}

