#pragma once

#include "Bitmap.h"
#include <cstring>

namespace bmfont_export {

irect_t clampBounds(const irect_t& a, const irect_t& b) {
    const int l = a.x > b.x ? a.x : b.x;
    const int t = a.y > b.y ? a.y : b.y;
    const int r = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    const int bo = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    return {{l, t, r - l, bo - t}};
}

void clipRects(const irect_t& src_bounds, const irect_t& dest_bounds,
               irect_t& src_rect, irect_t& dest_rect) {
    const irect_t src_rc = clampBounds(src_bounds, src_rect);
    const irect_t dest_rc = clampBounds(dest_bounds, dest_rect);
    src_rect = dest_rc;
    src_rect.x += src_rc.x - dest_rect.x;
    src_rect.y += src_rc.y - dest_rect.y;
    dest_rect = dest_rc;
}

inline uint32_t get_pixel_unsafe(const ek_bitmap bitmap, int x, int y) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    return bitmap.pixels[y * bitmap.w + x];
}

inline void set_pixel_unsafe(ek_bitmap bitmap, int x, int y, uint32_t pixel) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    bitmap.pixels[y * bitmap.w + x] = pixel;
}

void blit(ek_bitmap dest, const ek_bitmap src) {
    EK_ASSERT(dest.w >= src.w);
    EK_ASSERT(dest.h >= src.h);
    EK_ASSERT(dest.pixels);
    EK_ASSERT(src.pixels);
    for (int y = 0; y < src.h; ++y) {
        const auto* src_row = ek_bitmap_row(src, y);
        auto* dst_row = ek_bitmap_row(dest, y);
        for (int x = 0; x < src.w; ++x) {
            auto* d = dst_row + x;
            const auto* s = src_row + x;
            const auto a = s->a;
            if (a == 0) {
            } else if (a == 0xFFu) {
                *d = *s;
            } else {
                const auto alpha_inv = (0xFFu - a) * 258u;
                //t->r = (r+1 + (r >> 8)) >> 8; // fast way to divide by 255
                d->a = u8_add_sat(s->a, (uint8_t)((d->a * alpha_inv) >> 16u));
                d->r = u8_add_sat(s->r, (uint8_t)((d->r * alpha_inv) >> 16u));
                d->g = u8_add_sat(s->g, (uint8_t)((d->g * alpha_inv) >> 16u));
                d->b = u8_add_sat(s->b, (uint8_t)((d->b * alpha_inv) >> 16u));
            }
        }
    }
}

void convert_a8_to_argb32pma(uint8_t const* source_a8_buf,
                             uint32_t* dest_argb32_buf,
                             int pixels_count) {
    for (int i = 0; i < pixels_count; ++i) {
        const uint32_t a = *source_a8_buf;
        *dest_argb32_buf = (a << 24) | (a << 16) | (a << 8) | a;
        ++dest_argb32_buf;
        ++source_a8_buf;
    }
}

void convert_a8_to_argb32(uint8_t const* source_a8_buf,
                             uint32_t* dest_argb32_buf,
                             int pixels_count) {
    for (int i = 0; i < pixels_count; ++i) {
        const uint32_t a = *source_a8_buf;
        *dest_argb32_buf = (a << 24) | 0xFFFFFF;
        ++dest_argb32_buf;
        ++source_a8_buf;
    }
}

}