#ifndef EK_BITMAP_H
#define EK_BITMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <ek/math.h>

// expose stb_image API
#include <stb/stb_image.h>

#ifdef __cplusplus
extern "C" {
#endif

// bitmap memory container, could be passed by value, only pixels are heap allocated
typedef struct bitmap_t {
    int w;
    int h;
    color_t* pixels;
} bitmap_t;

void bitmap_alloc(bitmap_t* bitmap, int width, int height);

void bitmap_copy(bitmap_t dest, bitmap_t src);

void bitmap_clone(bitmap_t* dest, bitmap_t src);

void bitmap_free(bitmap_t* bitmap);

inline static color_t* bitmap_row(const bitmap_t bitmap, int y) {
    return bitmap.pixels + (bitmap.w * y);
}

// swizzle XYZW <-> XWZY, useful to convert between RGBA and BGRA pixel formats
inline static uint32_t swizzle_xwzy_u8(uint32_t a) {
    return (a & 0xFF00FF00u) | (((a << 16u) | (a >> 16u)) & 0x00FF00FFu);
}

void bitmap_swizzle_xwzy(bitmap_t* bitmap);

void bitmap_fill(bitmap_t* bitmap, color_t color);

void bitmap_blur_gray(uint8_t* data, int width, int height, int stride, float radius, int iterations, int strength);

// multiply all color components by alpha:
// r' = r * a
void bitmap_premultiply(bitmap_t* bitmap);

// convert back all color components using alpha:
// r = r' / a
void bitmap_unpremultiply(bitmap_t* bitmap);

void bitmap_decode(bitmap_t* bitmap, const void* data, uint32_t size, bool pma);

// blit one on another
void bitmap_blit(bitmap_t dest, bitmap_t src);

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
void bitmap_blit_copy_ccw90(bitmap_t* dest, int dx, int dy,
                            const bitmap_t* src, int sx, int sy, int sw, int sh);

void bitmap_blit_copy(bitmap_t* dest, int dx, int dy,
                      const bitmap_t* src, int sx, int sy, int sw, int sh);

#ifdef __cplusplus
}
#endif

#endif // EK_BITMAP_H
