#pragma once

#include "Base64.hpp"
#include <cstdint>

namespace ek::base64 {

// the general idea from
// https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.c

const uint8_t decoder[256] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

uint32_t getDecodedMaxSize(uint32_t string_length) {
    return ((string_length + 3) / 4) * 3;
}

uint32_t decode(uint8_t* out_buffer, const char* str, uint32_t chars_to_decode) {
    const auto* in = reinterpret_cast<const uint8_t*>(str);
    auto* p = out_buffer;

    while (chars_to_decode > 4) {
        const auto i0 = decoder[*(in++)];
        const auto i1 = decoder[*(in++)];
        const auto i2 = decoder[*(in++)];
        const auto i3 = decoder[*(in++)];
        *(p++) = (i0 << 2u) | (i1 >> 4u);
        *(p++) = (i1 << 4u) | (i2 >> 2u);
        *(p++) = (i2 << 6u) | i3;
        chars_to_decode -= 4;
    }
    // Note: (chars_to_decode == 1) would be an error, so just ignore that case
    if (chars_to_decode > 1) {
        const auto i0 = decoder[*(in++)];
        const auto i1 = decoder[*(in++)];
        *(p++) = (i0 << 2u) | (i1 >> 4u);
        if (chars_to_decode > 2) {
            const auto i2 = decoder[*(in++)];
            if (i2 < 64) {
                *(p++) = (i1 << 4u) | (i2 >> 2u);
                if (chars_to_decode > 3) {
                    const auto i3 = decoder[*in];
                    if (i3 < 64) {
                        *(p++) = (i2 << 6u) | i3;
                    }
                }
            }
        }
    }

    return p - out_buffer;
}

const char alphabet[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

// returns 64-based string length required to store 'bytes_count' bytes
uint32_t getEncodedMaxSize(uint32_t bytes_count) {
    return (bytes_count + 2) / 3 * 4;
}

uint32_t encode(char* out_str, const uint8_t* bytes, uint32_t bytes_count) {
    char* p = out_str;
    const uint8_t* in = bytes;
    while (bytes_count > 2) {
        const uint8_t i0 = *(in++);
        const uint8_t i1 = *(in++);
        const uint8_t i2 = *(in++);
        *(p++) = alphabet[i0 >> 2];
        *(p++) = alphabet[((i0 & 0x03) << 4) | (i1 >> 4)];
        *(p++) = alphabet[((i1 & 0x0F) << 2) | (i2 >> 6)];
        *(p++) = alphabet[i2 & 0x3F];
        bytes_count -= 3;
    }
    if (bytes_count > 0) {
        const uint8_t i0 = *(in++);
        *(p++) = alphabet[i0 >> 2];
        if (bytes_count == 1) {
            *(p++) = alphabet[(i0 & 0x03) << 4];
            *(p++) = '=';
        } else {
            const uint8_t i1 = *in;
            *(p++) = alphabet[((i0 & 0x03) << 4) | (i1 >> 4)];
            *(p++) = alphabet[(i1 & 0x0F) << 2];
        }
        *(p++) = '=';
    }

    return p - out_str;
}

}