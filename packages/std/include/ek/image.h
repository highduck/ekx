#ifndef EK_IMAGE_H
#define EK_IMAGE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ek_image {
    uint32_t w;
    uint32_t h;
    uint32_t* pixels;
} ek_image;

void ek_image_alloc(ek_image* img, int width, int height);
void ek_image_free(ek_image* img);

#define EK_IMAGE_SWAP_RB(pixel) (((pixel) & 0xFF00FF00u) | (((pixel) >> 16u) & 0xFFu) | (((pixel) & 0xFFu) << 16u))

void ek_image_swap_rb(ek_image* img);

void ek_image_fill(ek_image* img, uint32_t color);

void ek_image_blur_fast_a8(uint8_t* data, int width, int height, int stride, float radius, int iterations, int strength);

// multiply all color components by alpha:
// r' = r * a
void ek_image_premultiply(ek_image* img);

// convert back all color components using alpha:
// r = r' / a
void ek_image_un_premultiply(ek_image* img);

void ek_image_decode(ek_image* img, const void* data, uint32_t size, bool pma);

#ifdef __cplusplus
}
#endif

#endif // EK_IMAGE_H
