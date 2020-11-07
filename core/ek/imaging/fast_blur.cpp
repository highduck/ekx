#include <cmath>
#include <cassert>
#include "fast_blur.hpp"

// Based on Exponential blur, Jani Huhtanen, 2006

#define APREC 16
#define ZPREC 7

inline void blurCols(uint8_t* data, int width, int height, int stride, int alpha) {
    int x, y;
    for (y = 0; y < height; y++) {
        int z = 0; // force zero border
        for (x = 1; x < width; x++) {
            z += (alpha * (((int) (data[x]) << ZPREC) - z)) >> APREC;
            data[x] = (uint8_t) (z >> ZPREC);
        }
        data[width - 1] = 0; // force zero border
        z = 0;
        for (x = width - 2; x >= 0; x--) {
            z += (alpha * (((int) (data[x]) << ZPREC) - z)) >> APREC;
            data[x] = (uint8_t) (z >> ZPREC);
        }
        data[0] = 0; // force zero border
        data += stride;
    }
}

inline void blurRows(uint8_t* data, int width, int height, int stride, int alpha) {
    int x, y;
    for (x = 0; x < width; x++) {
        int z = 0; // force zero border
        for (y = stride; y < height * stride; y += stride) {
            z += (alpha * (((int) (data[y]) << ZPREC) - z)) >> APREC;
            data[y] = (uint8_t) (z >> ZPREC);
        }
        data[(height - 1) * stride] = 0; // force zero border
        z = 0;
        for (y = (height - 2) * stride; y >= 0; y -= stride) {
            z += (alpha * (((int) (data[y]) << ZPREC) - z)) >> APREC;
            data[y] = (uint8_t) (z >> ZPREC);
        }
        data[0] = 0; // force zero border
        data++;
    }
}

void expBlurAlpha(uint8_t* data, int width, int height, int stride, float radius, int iterations) {
    assert(radius >= 1.0f);
    // Calculate the alpha such that 90% of the kernel is within the radius. (Kernel extends to infinity)
    float sigma = radius * 0.57735f; // 1 / sqrt(3)
    int alpha = (int) ((1 << APREC) * (1.0f - expf(-2.3f / (sigma + 1.0f))));
    for (int i = 0; i < iterations; ++i) {
        blurRows(data, width, height, stride, alpha);
        blurCols(data, width, height, stride, alpha);
    }
}

inline uint8_t shiftSat(uint8_t x, uint8_t sh) {
    uint32_t s = x << sh;
    return s > 0xFF ? 0xFF : uint8_t(s);
}

void saturateAlpha(uint8_t* data, int width, int height, int stride, int shift) {
    for (int y = 0; y < height; ++y) {
        uint8_t* it = data;
        const uint8_t* end = data + width;
        for (; it != end; ++it) {
            *it = shiftSat(*it, shift);
        }
        data += stride;
    }
}

namespace ek {

void fastBlurA8(uint8_t* data, int width, int height, int stride, float radius, int iterations, int strength) {
    assert(width > 0 && height > 0 && stride > 0);
    if (radius >= 1.0f && iterations > 0) {
        if (iterations > 3) {
            iterations = 3;
        }
        if (radius > 255.0f) {
            radius = 255.0f;
        }
        expBlurAlpha(data, width, height, stride, radius, iterations);
    }
    if (strength > 0) {
        if (strength > 7) {
            strength = 7;
        }
        saturateAlpha(data, width, height, stride, strength);
    }
}

}