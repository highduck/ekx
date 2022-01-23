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