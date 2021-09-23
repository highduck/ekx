#pragma once

#include "common.h"
#include <cstdlib>
#include <cstring>

namespace sprite_packer {

class Bitmap {
public:
    uint32_t* data = nullptr;
    int w = 0;
    int h = 0;

    Bitmap() = default;

    Bitmap(const Bitmap& bitmap) : data{nullptr}, w{bitmap.w}, h{bitmap.h} {
        if (bitmap.data) {
            data = (uint32_t*) malloc(w * h * 4);
            memcpy(data, bitmap.data, w * h * 4);
        }
    }

    Bitmap(int width, int height) : data{nullptr}, w{width}, h{height} {
        data = (uint32_t*) calloc(1, width * height * 4);
    }

    Bitmap(void* pixels, int width, int height) : data{(uint32_t*) pixels}, w{width}, h{height} {
    }

    ~Bitmap() {
        free(data);
        data = nullptr;
    }
};

inline RectI clampBounds(const RectI& a, const RectI& b) {
    const int l = a.x > b.x ? a.x : b.x;
    const int t = a.y > b.y ? a.y : b.y;
    const int r = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    const int bo = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    return {l, t, r - l, bo - t};
}

inline void clipRects(const RectI& src_bounds, const RectI& dest_bounds,
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

/// 1 2 3 4
/// 1 2 3 4

//  4 4
//  3 3
//  2 2
//  1 1

inline void copyPixels_CCW_90(Bitmap& dest, int dx, int dy,
                              const Bitmap& src, int sx, int sy, int sw, int sh) {
    RectI dest_rc{dx, dy, /* swap w/h here */ sh, sw};
    RectI src_rc{sx, sy, sw, sh};
    clipRects({0, 0, src.w, src.h},
              {0, 0, dest.w, dest.h},
              src_rc, dest_rc);
    src_rc = {
            src_rc.x + dest_rc.x - dx,
            src_rc.y + dest_rc.y - dy,
            dest_rc.h,
            dest_rc.w
    };

    for (int32_t y = 0; y < src_rc.h; ++y) {
        for (int32_t x = 0; x < src_rc.w; ++x) {
            const auto pixel = get_pixel_unsafe(src, src_rc.x + x, src_rc.y + y);
            const int tx = dest_rc.x + y;
            const int ty = dest_rc.y + src_rc.w - x;
            set_pixel_unsafe(dest, tx, ty, pixel);
        }
    }
}

inline void copyPixels(Bitmap& dest, int dx, int dy,
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

}