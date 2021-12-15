#include "sprpk_image.h"

#include "sprpk_stb_impl.c.h"

#include <stdlib.h>
#include <string.h>

void ek_bitmap_alloc(ek_image* img, int w, int h) {
    img->w = w;
    img->h = h;
    img->pixels = (uint32_t*) calloc(1, w * h * 4);
}

void ek_bitmap_free(ek_image* img) {
    free(img->pixels);
    img->pixels = 0;
}

ek_img_rect clampBounds(const ek_img_rect a, const ek_img_rect b) {
    const int l = a.x > b.x ? a.x : b.x;
    const int t = a.y > b.y ? a.y : b.y;
    const int r = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    const int bo = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    return (ek_img_rect){l, t, r - l, bo - t};
}

void clipRects(const ek_img_rect src_bounds, const ek_img_rect dest_bounds,
               ek_img_rect* src_rect, ek_img_rect* dest_rect) {
    const ek_img_rect src_rc = clampBounds(src_bounds, *src_rect);
    const ek_img_rect dest_rc = clampBounds(dest_bounds, *dest_rect);
    src_rect->x = src_rc.x + dest_rc.x - dest_rect->x;
    src_rect->y = src_rc.y + dest_rc.y - dest_rect->y;
    src_rect->w = dest_rc.w;
    src_rect->h = dest_rc.h;
    *dest_rect = dest_rc;
}

uint32_t get_pixel_unsafe(const ek_image* image, int x, int y) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    return image->pixels[y * image->w + x];
}

void set_pixel_unsafe(ek_image* image, int x, int y, uint32_t pixel) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    image->pixels[y * image->w + x] = pixel;
}

/// 1 2 3 4
/// 1 2 3 4

//  4 4
//  3 3
//  2 2
//  1 1

void copyPixels_CCW_90(ek_image* dest, int dx, int dy,
                       const ek_image* src, int sx, int sy, int sw, int sh) {
    ek_img_rect dest_rc = {dx, dy, /* swap w/h here */ sh, sw};
    ek_img_rect src_rc = {sx, sy, sw, sh};
    clipRects((ek_img_rect){0, 0, src->w, src->h},
              (ek_img_rect){0, 0, dest->w, dest->h},
              &src_rc, &dest_rc);
    src_rc = (ek_img_rect){
            src_rc.x + dest_rc.x - dx,
            src_rc.y + dest_rc.y - dy,
            dest_rc.h,
            dest_rc.w
    };

    for (int32_t y = 0; y < src_rc.h; ++y) {
        for (int32_t x = 0; x < src_rc.w; ++x) {
            const uint32_t pixel = get_pixel_unsafe(src, src_rc.x + x, src_rc.y + y);
            const int tx = dest_rc.x + y;
            const int ty = dest_rc.y + src_rc.w - x;
            set_pixel_unsafe(dest, tx, ty, pixel);
        }
    }
}

void copyPixels(ek_image* dest, int dx, int dy,
                const ek_image* src, int sx, int sy, int sw, int sh) {
    ek_img_rect dest_rc = {dx, dy, sw, sh};
    ek_img_rect src_rc = {sx, sy, sw, sh};
    clipRects((ek_img_rect){0, 0, src->w, src->h},
              (ek_img_rect){0, 0, dest->w, dest->h},
              &src_rc, &dest_rc);

    for (int y = 0; y < src_rc.h; ++y) {
        for (int x = 0; x < src_rc.w; ++x) {
            const uint32_t pixel = get_pixel_unsafe(src, src_rc.x + x, src_rc.y + y);
            set_pixel_unsafe(dest, dest_rc.x + x, dest_rc.y + y, pixel);
        }
    }
}

void sprite_pack_image_save(const ek_image* bitmap, const char* path, uint32_t format_flags) {
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
            snprintf(path_buf, 1024, "%s.jpg", path);
            stbi_write_jpg(path_buf, w, h, 3, buffer_rgb, 90);
            snprintf(path_buf, 1024, "%s.a.jpg", path);
            stbi_write_jpg(path_buf, w, h, 1, buffer_alpha, 90);

            free(buffer_rgb);
            free(buffer_alpha);
        } else {
            auto* buffer = (uint8_t*) malloc(pixels_count * 3);
            auto* buffer_rgb = buffer;
            auto* buffer_rgba = (uint8_t*) bitmap->pixels;

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

