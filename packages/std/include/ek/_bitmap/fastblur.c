#include <ek/bitmap.h>
#include <ek/assert.h>

#include <math.h>

// Exponential blur, Jani Huhtanen, 2006

#ifdef __cplusplus
extern "C" {
#endif

#define APREC 12
#define ZPREC 10

static void blur_cols(uint8_t* data, int width, int height, int stride, int alpha) {
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

static void blur_rows(uint8_t* data, int width, int height, int stride, int alpha) {
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

static void exp_blur_alpha(uint8_t* data, int width, int height, int stride, float radius, int iterations) {
    EK_ASSERT(radius >= 1.0f && iterations > 0);
    float sigma = radius * (1.0f / sqrtf((float) iterations)); // 1 / sqrt(3)
    int alpha = (int) ((1 << APREC) * (1.0f - powf((float) iterations * 5.0f / 255.0f, 1.0f / sigma)));
    for (int i = 0; i < iterations; ++i) {
        blur_rows(data, width, height, stride, alpha);
        blur_cols(data, width, height, stride, alpha);
    }
}

static uint8_t shl_saturate(int x, int bits) {
    x <<= bits;
    return x > 0xFF ? 0xFF : (uint8_t) x;
}

static void saturate_alpha(uint8_t* data, int width, int height, int stride, int shift) {
    for (int y = 0; y < height; ++y) {
        uint8_t* it = data;
        const uint8_t* end = data + width;
        for (; it != end; ++it) {
            *it = shl_saturate(*it, shift);
        }
        data += stride;
    }
}

void bitmap_blur_gray(uint8_t* data, int width, int height, int stride, float radius, int iterations, int strength) {
    EK_ASSERT(width > 0 && height > 0 && stride > 0);
    if (radius >= 1.0f && iterations > 0) {
        if (iterations > 3) {
            iterations = 3;
        }
        if (radius > 255.0f) {
            radius = 255.0f;
        }
        exp_blur_alpha(data, width, height, stride, radius, iterations);
    }
    if (strength > 0) {
        if (strength > 7) {
            strength = 7;
        }
        saturate_alpha(data, width, height, stride, strength);
    }
}

#ifdef __cplusplus
}
#endif
