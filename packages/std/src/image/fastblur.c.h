#include <ek/image.h>
#include <ek/assert.h>

#include <math.h>

// Exponential blur, Jani Huhtanen, 2006

#define APREC 12
#define ZPREC 10

static void blurCols(uint8_t* data, int width, int height, int stride, int alpha) {
    int x, y;
    for (y = 0; y < height; y++) {
        int z = 0; // force zero border
        for (x = 1; x < width; x++) {
            z += (alpha * (((int) (data[x]) << ZPREC) - (z >> APREC)));
            data[x] = (uint8_t) (z >> (ZPREC + APREC));
        }
//        data[width - 1] = 0; // force zero border
        z = 0;
        for (x = width - 2; x >= 0; x--) {
            z += (alpha * (((int) (data[x]) << ZPREC) - (z >> APREC)));
            data[x] = (uint8_t) (z >> (ZPREC + APREC));
        }
//        data[0] = 0; // force zero border
        data += stride;
    }
}

static void blurRows(uint8_t* data, int width, int height, int stride, int alpha) {
    for (int x = 0; x < width; x++) {
        int z = 0; // force zero border
        for (int y = 1; y < height * stride; y += stride) {
            z += alpha * (((int) (data[y]) << ZPREC) - (z >> APREC));
            data[y] = (uint8_t) (z >> (ZPREC + APREC));
        }
        //data[(height - 1) * stride] = 0; // force zero border
        z = 0;
        for (int y = (height - 2) * stride; y >= 0; y -= stride) {
            z += (alpha * (((int) (data[y]) << ZPREC) - (z >> APREC)));
            data[y] = (uint8_t) (z >> (ZPREC + APREC));
        }
        //data[0] = 0; // force zero border
        data++;
    }
}

static void expBlurAlpha(uint8_t* data, int width, int height, int stride, float radius, int iterations) {
    EK_ASSERT(radius >= 1.0f && iterations > 0);
    float sigma = radius * (1.0f / sqrtf(iterations)); // 1 / sqrt(3)
    int alpha = (int) ((1 << APREC) * (1.0f - powf(iterations * 5.0f / 255.0f, 1.0f / sigma)));
    for (int i = 0; i < iterations; ++i) {
        blurRows(data, width, height, stride, alpha);
        blurCols(data, width, height, stride, alpha);
    }
}

static uint8_t shiftSat(int x, int sh) {
    x <<= sh;
    return x > 0xFF ? 0xFF : (uint8_t)x;
}

static void saturateAlpha(uint8_t* data, int width, int height, int stride, int shift) {
    for (int y = 0; y < height; ++y) {
        uint8_t* it = data;
        const uint8_t* end = data + width;
        for (; it != end; ++it) {
            *it = shiftSat(*it, shift);
        }
        data += stride;
    }
}

void ek_image_blur_fast_a8(uint8_t* data, int width, int height, int stride, float radius, int iterations, int strength) {
    EK_ASSERT(width > 0 && height > 0 && stride > 0);
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