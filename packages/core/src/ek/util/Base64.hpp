#pragma once

#include <cstdint>

namespace ek {

// returns base64 data length required to store `length` bytes
inline uint32_t base64_encodedMaxSize(uint32_t length) {
    return ((length + 2) / 3) * 4;
}

inline uint32_t base64_decodedMaxSize(uint32_t length){
    return ((length + 3) / 4) * 3;
}

uint32_t base64_encode(void* dst, uint32_t dstMaxSize, const void* src, uint32_t srcSize);

uint32_t base64_decode(void* dst, uint32_t dstMaxSize, const void* src, uint32_t srcSize);

}

