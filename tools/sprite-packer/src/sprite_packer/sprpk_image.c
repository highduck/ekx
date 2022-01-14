#include "sprpk_image.h"

#include "sprpk_stb_impl.c.h"
#include "ek/print.h"

#include <stdlib.h>

void sprite_pack_image_save(const ek_bitmap* bitmap, const char* path, uint32_t format_flags) {
    uint32_t alpha = format_flags & SPRITE_PACK_ALPHA;
    if(format_flags & SPRITE_PACK_PNG) {
        stbi_write_png_compression_level = 10;
        stbi_write_force_png_filter = 0;

        int w = bitmap->w;
        int h = bitmap->h;
        int pixels_count = w * h;

        if (alpha) {
            stbi_write_png(path, w, h, 4, bitmap->pixels, 4 * w);
        } else {
            uint8_t* buffer = (uint8_t*) malloc(pixels_count * 3);
            uint8_t* buffer_rgb = buffer;
            uint8_t* buffer_rgba = (uint8_t*) bitmap->pixels;

            for (int i = 0; i < pixels_count; ++i) {
                buffer_rgb[0] = buffer_rgba[0];
                buffer_rgb[1] = buffer_rgba[1];
                buffer_rgb[2] = buffer_rgba[2];
                buffer_rgba += 4;
                buffer_rgb += 3;
            }

            stbi_write_png(path, w, h, 3, buffer, 3 * w);
            free(buffer);
        }
    }
    else if(format_flags & SPRITE_PACK_JPEG) {
        int w = bitmap->w;
        int h = bitmap->h;
        int pixels_count = w * h;

        if (alpha) {
            uint8_t* buffer_rgb = (uint8_t*) malloc(pixels_count * 3);
            uint8_t* buffer_alpha = (uint8_t*) malloc(pixels_count);
            uint8_t* buffer_rgba = (uint8_t*) bitmap->pixels;

            uint8_t* rgb = buffer_rgb;
            uint8_t* alphaMask = buffer_alpha;
            for (int i = 0; i < pixels_count; ++i) {
                rgb[0] = buffer_rgba[0];
                rgb[1] = buffer_rgba[1];
                rgb[2] = buffer_rgba[2];
                alphaMask[0] = buffer_rgba[3];
                buffer_rgba += 4;
                rgb += 3;
                alphaMask += 1;
            }

            char path_buf[1024];
            ek_snprintf(path_buf, 1024, "%s.jpg", path);
            stbi_write_jpg(path_buf, w, h, 3, buffer_rgb, 90);
            ek_snprintf(path_buf, 1024, "%s.a.jpg", path);
            stbi_write_jpg(path_buf, w, h, 1, buffer_alpha, 90);

            free(buffer_rgb);
            free(buffer_alpha);
        } else {
            uint8_t* buffer = (uint8_t*) malloc(pixels_count * 3);
            uint8_t* buffer_rgb = buffer;
            uint8_t* buffer_rgba = (uint8_t*) bitmap->pixels;

            for (int i = 0; i < pixels_count; ++i) {
                buffer_rgb[0] = buffer_rgba[0];
                buffer_rgb[1] = buffer_rgba[1];
                buffer_rgb[2] = buffer_rgba[2];
                buffer_rgba += 4;
                buffer_rgb += 3;
            }

            stbi_write_jpg(path, w, h, 3, buffer, 3 * w);
            free(buffer);
        }
    }
}

