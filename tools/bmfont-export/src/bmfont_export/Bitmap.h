#pragma once

#include <cstdint>
#include "ek/bitmap.h"

namespace bmfont_export {

// todo: remove
//class Bitmap {
//public:
//    uint32_t* data = nullptr;
//    int w = 0;
//    int h = 0;
//
//    Bitmap();
//
//    Bitmap(int width, int height);
//
//    ~Bitmap();
//
//    [[nodiscard]]
//    const color_t* row(int y) const;
//
//    color_t* row(int y);
//
//    void assign(const Bitmap& source);
//};

inline static color_t* ek_bitmap_row(const ek_bitmap bitmap, int y) {
    return (color_t*)bitmap.pixels + (bitmap.w * y);
}

void blit(ek_bitmap dest, const ek_bitmap src);

void convert_a8_to_argb32pma(uint8_t const* source_a8_buf, uint32_t* dest_argb32_buf, int pixels_count);

void convert_a8_to_argb32(uint8_t const* source_a8_buf,
                          uint32_t* dest_argb32_buf,
                          int pixels_count);
}
