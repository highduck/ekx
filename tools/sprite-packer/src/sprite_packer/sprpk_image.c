#include "sprpk_image.h"

#include <stdlib.h>
#include <string.h>

void ek_bitmap_alloc(ek_bitmap* img, int w, int h) {
    img->w = w;
    img->h = h;
    img->data = (uint32_t*) calloc(1, w * h * 4);
}

void ek_bitmap_free(ek_bitmap* img) {
    free(img->data);
    img->data = 0;
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

uint32_t get_pixel_unsafe(const ek_bitmap* image, int x, int y) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    return image->data[y * image->w + x];
}

void set_pixel_unsafe(ek_bitmap* image, int x, int y, uint32_t pixel) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    image->data[y * image->w + x] = pixel;
}

/// 1 2 3 4
/// 1 2 3 4

//  4 4
//  3 3
//  2 2
//  1 1

void copyPixels_CCW_90(ek_bitmap* dest, int dx, int dy,
                       const ek_bitmap* src, int sx, int sy, int sw, int sh) {
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

void copyPixels(ek_bitmap* dest, int dx, int dy,
                const ek_bitmap* src, int sx, int sy, int sw, int sh) {
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
