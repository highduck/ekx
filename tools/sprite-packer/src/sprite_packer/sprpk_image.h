#ifndef SPRITE_PACKER_IMAGE_H
#define SPRITE_PACKER_IMAGE_H

#include <stdint.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <ek/math.h>
#include <ek/bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

void clip_rects(irect_t src_bounds, irect_t dest_bounds,
                irect_t* src_rect, irect_t* dest_rect);

uint32_t get_pixel_unsafe(const ek_bitmap* image, int x, int y);

void set_pixel_unsafe(ek_bitmap* image, int x, int y, uint32_t pixel);

/// 1 2 3 4
/// 1 2 3 4

//  4 4
//  3 3
//  2 2
//  1 1

void copy_pixels_ccw_90(ek_bitmap* dest, int dx, int dy,
                        const ek_bitmap* src, int sx, int sy, int sw, int sh);

void copy_pixels(ek_bitmap* dest, int dx, int dy,
                 const ek_bitmap* src, int sx, int sy, int sw, int sh);

typedef enum sprite_pack_format {
    SPRITE_PACK_AUTO = 0,
    SPRITE_PACK_BMP = 1,
    SPRITE_PACK_PNG = 2,
    SPRITE_PACK_JPEG = 3,
    SPRITE_PACK_ALPHA = 0x10
} sprite_pack_format;

void sprite_pack_image_save(const ek_bitmap* bitmap, const char* path, uint32_t format_flags);


#ifdef __cplusplus
}
#endif

#endif // SPRITE_PACKER_IMAGE_H
