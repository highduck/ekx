#include <ek/bitmap.h>
#include <ek/log.h>
#include <ek/assert.h>

#ifdef __cplusplus
extern "C" {
#endif

void ek_bitmap_alloc(ek_bitmap* bitmap, int width, int height) {
    EK_ASSERT(bitmap != 0);
    EK_ASSERT(width > 0);
    EK_ASSERT(height > 0);
    free(bitmap->pixels);
    bitmap->w = width;
    bitmap->h = height;
    bitmap->pixels = calloc(1, width * height * 4);
}

void ek_bitmap_free(ek_bitmap* bitmap) {
    free(bitmap->pixels);
    bitmap->pixels = NULL;
}

void ek_bitmap_swizzle_xwzy(ek_bitmap* bitmap) {
    uint32_t* it = bitmap->pixels;
    const uint32_t* end = it + (bitmap->w * bitmap->h);
    while (it != end) {
        *it = swizzle_xwzy_byte4(*it);
        ++it;
    }
}

void ek_bitmap_fill(ek_bitmap* bitmap, uint32_t color) {
    uint32_t* pixels = bitmap->pixels;
    const int size = bitmap->w * bitmap->h;
    for (int i = 0; i < size; ++i) {
        pixels[i] = color;
    }
}

typedef union ek_pix_ {
    uint32_t u32;

    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
} ek_pix_;

void ek_bitmap_premultiply(ek_bitmap* bitmap) {
    ek_pix_* it = (ek_pix_*) bitmap->pixels;
    const ek_pix_* end = it + (bitmap->w * bitmap->h);

    while (it < end) {
        const uint8_t a = it->a;
        if (!a) {
            it->u32 = 0;
        } else if (a ^ 0xFF) {
            it->r = ((uint16_t)it->r * a) / 0xFFu;
            it->g = ((uint16_t)it->g * a) / 0xFFu;
            it->b = ((uint16_t)it->b * a) / 0xFFu;
        }
        ++it;
    }
}

inline static uint8_t saturate_u8(uint16_t v) {
    return v < 255 ? v : 255;
}

void ek_bitmap_unpremultiply(ek_bitmap* bitmap) {
    ek_pix_* it = (ek_pix_*) bitmap->pixels;
    const ek_pix_* end = it + (bitmap->w * bitmap->h);

    while (it < end) {
        const uint8_t a = it->a;
        if (a && (a ^ 0xFF)) {
            const uint8_t half = a / 2;
            it->r = saturate_u8(((uint16_t)it->r * 0xFFu + half) / a);
            it->g = saturate_u8(((uint16_t)it->g * 0xFFu + half) / a);
            it->b = saturate_u8(((uint16_t)it->b * 0xFFu + half) / a);
        }
        ++it;
    }
}

#include <ek/log.h>
#include <ek/bitmap.h>

//extern stbi_uc* stbi_load_from_memory(stbi_uc const* buffer, int len, int* x, int* y, int* channels_in_file,
//                                      int desired_channels) ;
//
void ek_bitmap_decode(ek_bitmap* bitmap, const void* data, uint32_t size, bool pma) {
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
        bitmap->pixels = (uint32_t*) decoded;
        if (pma) {
            log_debug("decode image: premultiply alpha");
            EK_PROFILE_SCOPE("pma");
            ek_bitmap_premultiply(bitmap);
        }
    } else {
#ifndef NDEBUG
        log_error("bitmap decoding error: %s", stbi_failure_reason());
#endif
    }
    log_debug("decode bitmap: end");
}

#ifdef __cplusplus
}
#endif
