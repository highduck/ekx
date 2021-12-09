#pragma once

#include "image.hpp"
#include "../math/Vec.hpp"
#include "../math/Color32.hpp"
#include "../math/Rect.hpp"
#include <ek/assert.h>

namespace ek {

inline bool pixel_in_bounds(const image_t& image, const Vec2i& pos) {
    return pos.x >= 0
           && pos.y >= 0
           && pos.x < static_cast<int>(image.width())
           && pos.y < static_cast<int>(image.height());
}

inline abgr32_t get_pixel(const image_t& image, const Vec2i& pos) {
    if (pixel_in_bounds(image, pos)) {
        return *reinterpret_cast<const abgr32_t*>(image.data() + pos.y * image.stride() + pos.x * 4u);
    }
    return 0u;
}

inline void set_pixel(image_t& image, const Vec2i& pos, const abgr32_t pixel) {
    if (pixel_in_bounds(image, pos)) {
        *reinterpret_cast<abgr32_t*>(image.data() + pos.y * image.stride() + pos.x * 4u) = pixel;
    }
}

inline abgr32_t get_pixel_unsafe(const image_t& image, const Vec2i& pos) {
    EK_ASSERT(pixel_in_bounds(image, pos));
    return *reinterpret_cast<const abgr32_t*>(image.data() + pos.y * image.stride() + pos.x * 4u);
}

inline void set_pixel_unsafe(image_t& image, const Vec2i& pos, const abgr32_t pixel) {
    EK_ASSERT(pixel_in_bounds(image, pos));
    *reinterpret_cast<abgr32_t*>(image.data() + pos.y * image.stride() + pos.x * 4u) = pixel;
}

void copy_pixels_normal(image_t& dest, Vec2i dest_position,
                        const image_t& src, const Rect2i& rc);

void copy_pixels_ccw_90(image_t& dest, Vec2i dest_position,
                        const image_t& src, const Rect2i& rc);

void blit(image_t& dest, const image_t& src);

void convert_image_bgra_to_rgba(const image_t& src, image_t& dest);


void fill_image(image_t& image, abgr32_t color = 0x0u);

// multiply all color components by alpha:
// r' = r * a
void premultiplyAlpha(image_t& image);

// convert back all color components using alpha:
// r = r' / a
void undoPremultiplyAlpha(image_t& image);

}

