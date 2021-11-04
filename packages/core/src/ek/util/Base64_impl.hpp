#pragma once

#include "Base64.hpp"
#include <cstdint>

namespace ek {

// the general idea from
// https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.c

uint32_t base64_decode(void* dst, uint32_t dstMaxSize, const void* src, uint32_t srcSize) {
    // TODO: check `dstMaxSize` for overflow
    (void)dstMaxSize;

    static const uint8_t decoder[256] = {
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

    const auto* in = (const uint8_t*)src;
    auto* p = (uint8_t*)dst;

    while (srcSize > 4) {
        const auto i0 = decoder[*(in++)];
        const auto i1 = decoder[*(in++)];
        const auto i2 = decoder[*(in++)];
        const auto i3 = decoder[*(in++)];
        *(p++) = (i0 << 2u) | (i1 >> 4u);
        *(p++) = (i1 << 4u) | (i2 >> 2u);
        *(p++) = (i2 << 6u) | i3;
        srcSize -= 4;
    }
    // Note: (srcSize == 1) would be an error, so just ignore that case
    if (srcSize > 1) {
        const auto i0 = decoder[*(in++)];
        const auto i1 = decoder[*(in++)];
        *(p++) = (i0 << 2u) | (i1 >> 4u);
        if (srcSize > 2) {
            const auto i2 = decoder[*(in++)];
            if (i2 < 64) {
                *(p++) = (i1 << 4u) | (i2 >> 2u);
                if (srcSize > 3) {
                    const auto i3 = decoder[*in];
                    if (i3 < 64) {
                        *(p++) = (i2 << 6u) | i3;
                    }
                }
            }
        }
    }

    return p - (uint8_t*)dst;
}

uint32_t base64_encode(void* dst, uint32_t dstMaxSize, const void* src, uint32_t srcSize) {
    // TODO: check `dstMaxSize` for overflow
    (void)dstMaxSize;

    static const char alphabet[66] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/=";

    auto* p = (char*)dst;
    const auto* in = (const uint8_t*)src;
    while (srcSize > 2) {
        const uint8_t i0 = *(in++);
        const uint8_t i1 = *(in++);
        const uint8_t i2 = *(in++);
        *(p++) = alphabet[i0 >> 2];
        *(p++) = alphabet[((i0 & 0x03) << 4) | (i1 >> 4)];
        *(p++) = alphabet[((i1 & 0x0F) << 2) | (i2 >> 6)];
        *(p++) = alphabet[i2 & 0x3F];
        srcSize -= 3;
    }
    if (srcSize > 0) {
        const uint8_t i0 = *(in++);
        *(p++) = alphabet[i0 >> 2];
        if (srcSize == 1) {
            *(p++) = alphabet[(i0 & 0x03) << 4];
            *(p++) = alphabet[0x40];
        } else {
            const uint8_t i1 = *in;
            *(p++) = alphabet[((i0 & 0x03) << 4) | (i1 >> 4)];
            *(p++) = alphabet[(i1 & 0x0F) << 2];
        }
        *(p++) = alphabet[0x40];
    }

    return p - (const char*)dst;
}

}