#pragma once

#include "Bitmap.h"
#include <cstring>

namespace bmfont_export {

Bitmap::Bitmap() = default;

Bitmap::Bitmap(int width, int height) {
    data = (uint32_t*) malloc(width * height * 4);
    w = width;
    h = height;
}

Bitmap::~Bitmap() {
    free(data);
}

const Rgba* Bitmap::row(int y) const {
    return (const Rgba*) (data + w * y);
}

Rgba* Bitmap::row(int y) {
    return (Rgba*) (data + w * y);
}

void Bitmap::assign(const Bitmap& source) {
    memcpy(data, source.data, source.w * source.h * 4);
}

RectI clampBounds(const RectI& a, const RectI& b) {
    const int l = a.x > b.x ? a.x : b.x;
    const int t = a.y > b.y ? a.y : b.y;
    const int r = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    const int bo = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    return {l, t, r - l, bo - t};
}

void clipRects(const RectI& src_bounds, const RectI& dest_bounds,
               RectI& src_rect, RectI& dest_rect) {
    const RectI src_rc = clampBounds(src_bounds, src_rect);
    const RectI dest_rc = clampBounds(dest_bounds, dest_rect);
    src_rect = {
            src_rc.x + dest_rc.x - dest_rect.x,
            src_rc.y + dest_rc.y - dest_rect.y,
            dest_rc.w,
            dest_rc.h
    };
    dest_rect = dest_rc;
}

inline uint32_t get_pixel_unsafe(const Bitmap& image, int x, int y) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    return image.data[y * image.w + x];
}

inline void set_pixel_unsafe(Bitmap& image, int x, int y, uint32_t pixel) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    image.data[y * image.w + x] = pixel;
}

void copyPixels(Bitmap& dest, int dx, int dy,
                const Bitmap& src, int sx, int sy, int sw, int sh) {
    RectI dest_rc{dx, dy, sw, sh};
    RectI src_rc{sx, sy, sw, sh};
    clipRects({0, 0, src.w, src.h},
              {0, 0, dest.w, dest.h},
              src_rc, dest_rc);

    for (int y = 0; y < src_rc.h; ++y) {
        for (int x = 0; x < src_rc.w; ++x) {
            const auto pixel = get_pixel_unsafe(src, src_rc.x + x, src_rc.y + y);
            set_pixel_unsafe(dest, dest_rc.x + x, dest_rc.y + y, pixel);
        }
    }
}


void blit(Bitmap& dest, const Bitmap& src) {
    for (int y = 0; y < src.h; ++y) {
        const auto* src_row = src.row(y);
        auto* dst_row = dest.row(y);
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
                d->a = s->a + static_cast<uint8_t>((d->a * alpha_inv) >> 16u);
                d->r = s->r + static_cast<uint8_t>((d->r * alpha_inv) >> 16u);
                d->g = s->g + static_cast<uint8_t>((d->g * alpha_inv) >> 16u);
                d->b = s->b + static_cast<uint8_t>((d->b * alpha_inv) >> 16u);
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

void undoPremultiplyAlpha(Bitmap& bitmap) {
    auto* it = (Rgba*) bitmap.data;
    const auto* end = it + bitmap.w * bitmap.h;

    while (it < end) {
        const uint8_t a = it->a;
        if (a && (a ^ 0xFF)) {
            const uint8_t half = a / 2;
            it->r = std::min(255, (it->r * 0xFF + half) / a);
            it->g = std::min(255, (it->g * 0xFF + half) / a);
            it->b = std::min(255, (it->b * 0xFF + half) / a);
        }
        ++it;
    }
}

}