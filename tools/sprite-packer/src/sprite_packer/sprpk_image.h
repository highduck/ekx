#ifndef SPRITE_PACKER_IMAGE_H
#define SPRITE_PACKER_IMAGE_H

#include <stdint.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

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

typedef struct ek_image {
    int w;
    int h;
    uint32_t* pixels;
} ek_image;

void ek_bitmap_alloc(ek_image* img, int w, int h);

void ek_bitmap_free(ek_image* img);

ek_img_rect clampBounds(ek_img_rect a, ek_img_rect b);

void clipRects(ek_img_rect src_bounds, ek_img_rect dest_bounds,
               ek_img_rect* src_rect, ek_img_rect* dest_rect);

uint32_t get_pixel_unsafe(const ek_image* image, int x, int y);

void set_pixel_unsafe(ek_image* image, int x, int y, uint32_t pixel);

/// 1 2 3 4
/// 1 2 3 4

//  4 4
//  3 3
//  2 2
//  1 1

void copyPixels_CCW_90(ek_image* dest, int dx, int dy,
                       const ek_image* src, int sx, int sy, int sw, int sh);

void copyPixels(ek_image* dest, int dx, int dy,
                const ek_image* src, int sx, int sy, int sw, int sh);

typedef enum sprite_pack_format {
    SPRITE_PACK_AUTO = 0,
    SPRITE_PACK_BMP = 1,
    SPRITE_PACK_PNG = 2,
    SPRITE_PACK_JPEG = 3,
    SPRITE_PACK_ALPHA = 0x10
} sprite_pack_format;

void sprite_pack_image_save(const ek_image* bitmap, const char* path, uint32_t format_flags);


#ifdef __cplusplus
}
#endif

#endif // SPRITE_PACKER_IMAGE_H
