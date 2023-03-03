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

typedef enum sprite_pack_format {
    SPRITE_PACK_AUTO = 0,
    SPRITE_PACK_BMP = 1,
    SPRITE_PACK_PNG = 2,
    SPRITE_PACK_JPEG = 3,
    SPRITE_PACK_ALPHA = 0x10
} sprite_pack_format;

void sprite_pack_image_save(const bitmap_t* bitmap, const char* path, uint32_t format_flags);

#ifdef __cplusplus
}
#endif

#endif // SPRITE_PACKER_IMAGE_H
