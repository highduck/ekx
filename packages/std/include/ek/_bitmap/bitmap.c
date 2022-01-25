#include <ek/bitmap.h>
#include <ek/log.h>
#include <ek/assert.h>

#ifdef __cplusplus
extern "C" {
#endif

void bitmap_alloc(bitmap_t* bitmap, int width, int height) {
    EK_ASSERT(bitmap != 0);
    EK_ASSERT(width > 0);
    EK_ASSERT(height > 0);
    free(bitmap->pixels);
    bitmap->w = width;
    bitmap->h = height;
    bitmap->pixels = calloc(1, width * height * 4);
}

void bitmap_copy(bitmap_t dest, bitmap_t src) {
    EK_ASSERT(dest.w == src.w && dest.h == src.h);
    EK_ASSERT(dest.pixels);
    EK_ASSERT(src.pixels);
    uint32_t size = src.w * src.h * 4;
    memcpy(dest.pixels, src.pixels, size);
}

void bitmap_clone(bitmap_t* dest, bitmap_t src) {
    free(dest->pixels);
    dest->w = src.w;
    dest->h = src.h;
    uint32_t size = src.w * src.h * 4;
    dest->pixels = malloc(size);
    memcpy(dest->pixels, src.pixels, size);
}

void bitmap_free(bitmap_t* bitmap) {
    free(bitmap->pixels);
    bitmap->pixels = NULL;
}

void bitmap_swizzle_xwzy(bitmap_t* bitmap) {
    color_t* it = bitmap->pixels;
    const color_t* end = it + (bitmap->w * bitmap->h);
    while (it != end) {
        it->value = swizzle_xwzy_u8(it->value);
        ++it;
    }
}

void bitmap_fill(bitmap_t* bitmap, color_t color) {
    color_t* pixels = bitmap->pixels;
    const int size = bitmap->w * bitmap->h;
    for (int i = 0; i < size; ++i) {
        pixels[i] = color;
    }
}

void bitmap_premultiply(bitmap_t* bitmap) {
    color_t* it = (color_t*) bitmap->pixels;
    const color_t* end = it + (bitmap->w * bitmap->h);

    while (it < end) {
        const uint8_t a = it->a;
        if (!a) {
            it->value = 0;
        } else if (a ^ 0xFF) {
            it->r = ((uint16_t) it->r * a) / 0xFFu;
            it->g = ((uint16_t) it->g * a) / 0xFFu;
            it->b = ((uint16_t) it->b * a) / 0xFFu;
        }
        ++it;
    }
}

inline static uint8_t saturate_u8(uint16_t v) {
    return v < 255 ? v : 255;
}

void bitmap_unpremultiply(bitmap_t* bitmap) {
    color_t* it = (color_t*) bitmap->pixels;
    const color_t* end = it + (bitmap->w * bitmap->h);

    while (it < end) {
        const uint8_t a = it->a;
        if (a && (a ^ 0xFF)) {
            const uint8_t half = a / 2;
            it->r = saturate_u8(((uint16_t) it->r * 0xFFu + half) / a);
            it->g = saturate_u8(((uint16_t) it->g * 0xFFu + half) / a);
            it->b = saturate_u8(((uint16_t) it->b * 0xFFu + half) / a);
        }
        ++it;
    }
}

#include <ek/log.h>
#include <ek/bitmap.h>

//extern stbi_uc* stbi_load_from_memory(stbi_uc const* buffer, int len, int* x, int* y, int* channels_in_file,
//                                      int desired_channels) ;
//
void bitmap_decode(bitmap_t* bitmap, const void* data, uint32_t size, bool pma) {
    log_debug("decode bitmap: canvas_begin");
    EK_ASSERT(size > 0);
    EK_ASSERT(bitmap != 0);
    EK_ASSERT(bitmap->pixels == 0);

    int w = 0;
    int h = 0;
    int channels = 0;
    stbi_uc* decoded = stbi_load_from_memory((const uint8_t*) data,
                                             (int) size,
                                             &w, &h, &channels, 4);


    if (decoded) {
        bitmap->w = w;
        bitmap->h = h;
        bitmap->pixels = (color_t*) decoded;
        if (pma) {
            log_debug("decode image: premultiply alpha");
            EK_PROFILE_SCOPE("pma");
            bitmap_premultiply(bitmap);
        }
    } else {
#ifndef NDEBUG
        log_error("bitmap decoding error: %s", stbi_failure_reason());
#endif
    }
    log_debug("decode bitmap: end");
}


#include <ek/math.h>

irect_t irect_clamp_size(irect_t a, int w, int h) {
    const int x0 = MAX(a.x, 0);
    const int y0 = MAX(a.y, 0);
    const int x1 = MIN(RECT_R(a), w);
    const int y1 = MIN(RECT_B(a), h);
    return (irect_t) {{x0, y0, x1 - x0, y1 - y0}};
}

static void clip_rects(int src_w, int src_h, int dest_w, int dest_h,
                       irect_t* src_rect, irect_t* dest_rect) {
    const irect_t src_rc = irect_clamp_size(*src_rect, src_w, src_h);
    const irect_t dest_rc = irect_clamp_size(*dest_rect, dest_w, dest_h);
    src_rect->x = src_rc.x + dest_rc.x - dest_rect->x;
    src_rect->y = src_rc.y + dest_rc.y - dest_rect->y;
    src_rect->w = dest_rc.w;
    src_rect->h = dest_rc.h;
    *dest_rect = dest_rc;
}

static color_t get_pixel_unsafe(const bitmap_t* bitmap, int x, int y) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    return bitmap->pixels[y * bitmap->w + x];
}

static void set_pixel_unsafe(bitmap_t* bitmap, int x, int y, color_t pixel) {
    //EK_ASSERT(pixel_in_bounds(image, pos));
    bitmap->pixels[y * bitmap->w + x] = pixel;
}


void bitmap_blit_copy_ccw90(bitmap_t* dest, int dx, int dy,
                            const bitmap_t* src, int sx, int sy, int sw, int sh) {
    irect_t dest_rc = {{dx, dy, /* swap w/h here */ sh, sw}};
    irect_t src_rc = {{sx, sy, sw, sh}};
    clip_rects(src->w, src->h,
               dest->w, dest->h,
               &src_rc, &dest_rc);
    src_rc = (irect_t) {{
                                src_rc.x + dest_rc.x - dx,
                                src_rc.y + dest_rc.y - dy,
                                dest_rc.h,
                                dest_rc.w
                        }};

    for (int32_t y = 0; y < src_rc.h; ++y) {
        for (int32_t x = 0; x < src_rc.w; ++x) {
            const color_t pixel = get_pixel_unsafe(src, src_rc.x + x, src_rc.y + y);
            const int tx = dest_rc.x + y;
            const int ty = dest_rc.y + src_rc.w - x;
            set_pixel_unsafe(dest, tx, ty, pixel);
        }
    }
}

void bitmap_blit_copy(bitmap_t* dest, int dx, int dy,
                      const bitmap_t* src, int sx, int sy, int sw, int sh) {
    irect_t dest_rc = {{dx, dy, sw, sh}};
    irect_t src_rc = {{sx, sy, sw, sh}};
    clip_rects(src->w, src->h,
               dest->w, dest->h,
               &src_rc, &dest_rc);

    for (int y = 0; y < src_rc.h; ++y) {
        for (int x = 0; x < src_rc.w; ++x) {
            const color_t pixel = get_pixel_unsafe(src, src_rc.x + x, src_rc.y + y);
            set_pixel_unsafe(dest, dest_rc.x + x, dest_rc.y + y, pixel);
        }
    }
}

void bitmap_blit(bitmap_t dest, const bitmap_t src) {
    EK_ASSERT(dest.w >= src.w);
    EK_ASSERT(dest.h >= src.h);
    EK_ASSERT(dest.pixels);
    EK_ASSERT(src.pixels);
    for (int y = 0; y < src.h; ++y) {
        const color_t* src_row = bitmap_row(src, y);
        color_t* dst_row = bitmap_row(dest, y);
        for (int x = 0; x < src.w; ++x) {
            color_t* d = dst_row + x;
            const color_t* s = src_row + x;
            const uint8_t a = s->a;
            if (a == 0) {
            } else if (a == 0xFFu) {
                *d = *s;
            } else {
                const uint32_t alpha_inv = (0xFFu - a) * 258u;
                //t->r = (r+1 + (r >> 8)) >> 8; // fast way to divide by 255
                d->a = u8_add_sat(s->a, (uint8_t) ((d->a * alpha_inv) >> 16u));
                d->r = u8_add_sat(s->r, (uint8_t) ((d->r * alpha_inv) >> 16u));
                d->g = u8_add_sat(s->g, (uint8_t) ((d->g * alpha_inv) >> 16u));
                d->b = u8_add_sat(s->b, (uint8_t) ((d->b * alpha_inv) >> 16u));
            }
        }
    }
}

#ifdef __cplusplus
}
#endif
