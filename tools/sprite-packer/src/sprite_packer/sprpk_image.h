#ifndef SPRITE_PACKER_IMAGE_H
#define SPRITE_PACKER_IMAGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ek_img_rect {
    int x;
    int y;
    int w;
    int h;
} ek_img_rect;

typedef struct ek_img_rect_f {
    float x;
    float y;
    float w;
    float h;
} ek_img_rect_f;

typedef struct ek_bitmap {
    int w;
    int h;
    uint32_t* data;
} ek_bitmap;

void ek_bitmap_alloc(ek_bitmap* img, int w, int h);

void ek_bitmap_free(ek_bitmap* img);

ek_img_rect clampBounds(const ek_img_rect a, const ek_img_rect b);

void clipRects(const ek_img_rect src_bounds, const ek_img_rect dest_bounds,
               ek_img_rect* src_rect, ek_img_rect* dest_rect);

uint32_t get_pixel_unsafe(const ek_bitmap* image, int x, int y);

void set_pixel_unsafe(ek_bitmap* image, int x, int y, uint32_t pixel);

/// 1 2 3 4
/// 1 2 3 4

//  4 4
//  3 3
//  2 2
//  1 1

void copyPixels_CCW_90(ek_bitmap* dest, int dx, int dy,
                       const ek_bitmap* src, int sx, int sy, int sw, int sh);

void copyPixels(ek_bitmap* dest, int dx, int dy,
                const ek_bitmap* src, int sx, int sy, int sw, int sh);

#ifdef __cplusplus
}
#endif

#endif // SPRITE_PACKER_IMAGE_H
