#ifndef EK_BITMAP_H
#define EK_BITMAP_H

#include <stdint.h>
#include <stdbool.h>

// expose stb_image API
#include <stb/stb_image.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ek_bitmap {
    int w;
    int h;
    uint32_t* pixels;
} ek_bitmap;

void ek_bitmap_alloc(ek_bitmap* bitmap, int width, int height);

void ek_bitmap_free(ek_bitmap* bitmap);

// swizzle XYZW <-> XWZY, useful to convert between RGBA and BGRA pixel formats
inline static uint32_t swizzle_xwzy_byte4(uint32_t a) {
    return (a & 0xFF00FF00u) | (((a << 16u) | (a >> 16u)) & 0x00FF00FFu);
}

void ek_bitmap_swizzle_xwzy(ek_bitmap* bitmap);

void ek_bitmap_fill(ek_bitmap* bitmap, uint32_t color);

void ek_bitmap_blur_gray(uint8_t* data, int width, int height, int stride, float radius, int iterations, int strength);

// multiply all color components by alpha:
// r' = r * a
void ek_bitmap_premultiply(ek_bitmap* bitmap);

// convert back all color components using alpha:
// r = r' / a
void ek_bitmap_unpremultiply(ek_bitmap* bitmap);

void ek_bitmap_decode(ek_bitmap* bitmap, const void* data, uint32_t size, bool pma);


/**
 *
 *          Rotate CCW 90
 * -----------         -------
 * | 1 2 3 4 |  ---->  | 4 4 |
 * | 1 2 3 4 |         | 3 3 |
 * -----------         | 2 2 |
 *                     | 1 1 |
 *                     -------
 */
void bitmap_copy_ccw90(ek_bitmap* dest, int dx, int dy,
                       const ek_bitmap* src, int sx, int sy, int sw, int sh);

void bitmap_copy(ek_bitmap* dest, int dx, int dy,
                 const ek_bitmap* src, int sx, int sy, int sw, int sh);

#ifdef __cplusplus
}
#endif

#endif // EK_BITMAP_H
