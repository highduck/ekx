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

#define EK_PIXEL_SWAP_RB(pixel) (((pixel) & 0xFF00FF00u) | (((pixel) >> 16u) & 0xFFu) | (((pixel) & 0xFFu) << 16u))

void ek_bitmap_swap_rb(ek_bitmap* bitmap);

void ek_bitmap_fill(ek_bitmap* bitmap, uint32_t color);

void ek_bitmap_blur_gray(uint8_t* data, int width, int height, int stride, float radius, int iterations, int strength);

// multiply all color components by alpha:
// r' = r * a
void ek_bitmap_premultiply(ek_bitmap* bitmap);

// convert back all color components using alpha:
// r = r' / a
void ek_bitmap_un_premultiply(ek_bitmap* bitmap);

void ek_bitmap_decode(ek_bitmap* bitmap, const void* data, uint32_t size, bool pma);

#ifdef __cplusplus
}
#endif

#endif // EK_BITMAP_H
