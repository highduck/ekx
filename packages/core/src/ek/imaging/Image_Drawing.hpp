#include "drawing.hpp"

namespace ek {

void clip_rects(const Rect2i& src_bounds, const Rect2i& dest_bounds,
                Rect2i& src_rect, Rect2i& dest_rect) {
    const Rect2i src_rc = clamp_bounds(src_bounds, src_rect);
    const Rect2i dest_rc = clamp_bounds(dest_bounds, dest_rect);
    src_rect = {
            src_rc.position + dest_rc.position - dest_rect.position,
            dest_rc.size
    };
    dest_rect = dest_rc;
}

void copy_pixels_normal(image_t& dest, Vec2i dest_position,
                        const image_t& src, const Rect2i& src_rect) {
    Rect2i dest_rc = {dest_position, src_rect.size};
    Rect2i src_rc = src_rect;
    clip_rects(src.bounds<int>(), dest.bounds<int>(),
               src_rc, dest_rc);

    for (int32_t y = 0; y < src_rc.height; ++y) {
        for (int32_t x = 0; x < src_rc.width; ++x) {
            const auto pixel = get_pixel_unsafe(src, Vec2i{src_rc.x + x, src_rc.y + y});
            set_pixel_unsafe(dest, {dest_rc.x + x, dest_rc.y + y}, pixel);
        }
    }
}

/// 1 2 3 4
/// 1 2 3 4

//  4 4
//  3 3
//  2 2
//  1 1

void copy_pixels_ccw_90(image_t& dest, Vec2i dest_position, const image_t& src, const Rect2i& src_rect) {
    Rect2i src_rc = clamp_bounds(src.bounds<int>(), src_rect);
    Rect2i dest_rc = clamp_bounds(dest.bounds<int>(), {dest_position, {src_rc.height, src_rc.width}});
    src_rc = {
            src_rc.position + dest_rc.position - dest_position,
            {dest_rc.height, dest_rc.width}
    };

    for (int32_t y = 0; y < src_rc.height; ++y) {
        for (int32_t x = 0; x < src_rc.width; ++x) {
            const auto pixel = get_pixel_unsafe(src, Vec2i{src_rc.x + x, src_rc.y + y});
            const Vec2i dest_transformed_position{dest_rc.x + y, dest_rc.y + src_rc.width - x};
            set_pixel_unsafe(dest, dest_transformed_position, pixel);
        }
    }
}

// TODO: fast copy pixels without bounds checking
//void image_t::copy_pixels(const uint8_t* src, uint32_t src_stride, uint32_t src_height, uint32_t tx, uint32_t ty) {
//    for (uint32_t y = 0; y < src_height; ++y) {
//        std::copy_n(src + y * src_stride, src_stride, data_ + (ty + y) * stride() + tx * bytes_per_pixel_);
//    }
//}


void blit(image_t& dest, const image_t& src) {
    for (uint32_t y = 0; y < src.height(); ++y) {
        const auto* src_row = src.row(y);
        auto* dst_row = dest.row(y);
        for (uint32_t x = 0; x < src.width(); ++x) {
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

void convert_image_bgra_to_rgba(const image_t& src, image_t& dest) {
    auto* dest_pixels = reinterpret_cast<uint32_t*>(dest.data());
    const auto* src_pixels = reinterpret_cast<const uint32_t*>(src.data());
    const uint32_t size = src.width() * src.height();
    for (uint32_t i = 0; i < size; ++i) {
        const uint32_t c = src_pixels[i];
        dest_pixels[i] = (c & 0xFF00FF00u) | ((c >> 16u) & 0xFFu) | ((c & 0xFFu) << 16u);
    }
}


void fill_image(image_t& image, abgr32_t color) {
    const auto end = image.width() * image.height();
    auto* d = image.row<abgr32_t>(0);
    for (size_t i = 0; i < end; ++i) {
        *(d++) = color;
    }
}

//static inline int multiply_alpha (int alpha, int color)
//{
//    int temp = (alpha * color) + 0x80;
//    return ((temp + (temp >> 8)) >> 8);
//}

union Pix {
    uint32_t u32;

    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } rgba;
};

void premultiplyAlpha(image_t& image) {
    Pix* it = (Pix*)image.data();
    const uint32_t len = image.width() * image.height();

    for(uint32_t i = 0; i < len; ++i) {
        Pix p = it[i];
        const uint8_t a = p.rgba.a;
        if (!a) {
            it[i].u32 = 0;
        }
        else if(a ^ 0xFF) {
            p.rgba.r = (p.rgba.r * a) / 0xFF;
            p.rgba.g = (p.rgba.g * a) / 0xFF;
            p.rgba.b = (p.rgba.b * a) / 0xFF;
            it[i] = p;
        }
    }
}

void undoPremultiplyAlpha(image_t& bitmap) {
    auto* it = (abgr32_t*) bitmap.data();
    const auto* end = it + (bitmap.width() * bitmap.height());

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