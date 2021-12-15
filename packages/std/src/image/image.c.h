#include <ek/image.h>
#include <ek/log.h>
#include <ek/assert.h>

void ek_image_alloc(ek_image* img, int width, int height) {
    EK_ASSERT(img != 0);
    EK_ASSERT(width > 0);
    EK_ASSERT(height > 0);
    free(img->pixels);
    img->w = width;
    img->h = height;
    img->pixels = malloc(width * height * 4);
}

void ek_image_free(ek_image* img) {
    free(img->pixels);
    img->pixels = NULL;
}

void ek_image_swap_rb(ek_image* img) {
    uint32_t* pixels = img->pixels;
    const int size = img->w * img->h;
    for (int i = 0; i < size; ++i) {
        const uint32_t c = pixels[i];
        pixels[i] = EK_IMAGE_SWAP_RB(c);
    }
}

void ek_image_fill(ek_image* img, uint32_t color) {
    uint32_t* pixels = img->pixels;
    const int size = img->w * img->h;
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

void ek_image_premultiply(ek_image* img) {
    ek_pix_* it = (ek_pix_*) img->pixels;
    const ek_pix_* end = it + (img->w * img->h);

    while (it < end) {
        const uint8_t a = it->a;
        if (!a) {
            it->u32 = 0;
        } else if (a ^ 0xFF) {
            it->r = (it->r * a) / 0xFF;
            it->g = (it->g * a) / 0xFF;
            it->b = (it->b * a) / 0xFF;
        }
        ++it;
    }
}

inline static uint8_t saturate_u8(int v) {
    return v < 255 ? v : 255;
}

void ek_image_un_premultiply(ek_image* img) {
    ek_pix_* it = (ek_pix_*) img->pixels;
    const ek_pix_* end = it + (img->w * img->h);

    while (it < end) {
        const uint8_t a = it->a;
        if (a && (a ^ 0xFF)) {
            const uint8_t half = a / 2;
            it->r = saturate_u8((it->r * 0xFF + half) / a);
            it->g = saturate_u8((it->g * 0xFF + half) / a);
            it->b = saturate_u8((it->b * 0xFF + half) / a);
        }
        ++it;
    }
}

#include <ek/log.h>
#include <ek/image.h>

//extern stbi_uc* stbi_load_from_memory(stbi_uc const* buffer, int len, int* x, int* y, int* channels_in_file,
//                                      int desired_channels) ;
//
void ek_image_decode(ek_image* img, const void* data, uint32_t size, bool pma) {
    log_debug("decode image: begin");
    EK_ASSERT(size > 0);
    EK_ASSERT(img != 0);
    EK_ASSERT(img->pixels == 0);

    int w = 0;
    int h = 0;
    int channels = 0;
    stbi_uc* decoded = stbi_load_from_memory((const uint8_t*) data,
                                             (int) size,
                                             &w, &h, &channels, 4);


    if (decoded) {
        img->w = w;
        img->h = h;
        img->pixels = (uint32_t*) decoded;
        if (pma) {
            log_debug("decode image: premultiply alpha");
            EK_PROFILE_SCOPE("pma");
            ek_image_premultiply(img);
        }
    } else {
#ifndef NDEBUG
        log_error("image decoding error: %s", stbi_failure_reason());
#endif
    }
    log_debug("decode image: end");
}
