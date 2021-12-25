#include "ImageSet.hpp"
#include <pugixml.hpp>
#include <ek/assert.h>

#include <miniz.h>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define STBIW_ASSERT(e)   EK_ASSERT(e)

namespace {
inline unsigned char* iwcompress(unsigned char* data, int data_len, int* out_len, int quality) {
    // uber compression
    quality = 10;
    mz_ulong buflen = mz_compressBound(data_len);
    auto* buf = (unsigned char*) malloc(buflen);
    if (mz_compress2(buf, &buflen, data, data_len, quality)) {
        free(buf);
        return nullptr;
    }
    *out_len = (int) buflen;
    return buf;
}
}

#define STBIW_ZLIB_COMPRESS(a, b, c, d)  ::iwcompress(a,b,c,d)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"

#include <stb/stb_image_write.h>
#include <stb/stb_sprintf.h>

#pragma clang diagnostic pop

#endif

namespace ek {

/*** Save Image ***/

void ek_bitmap_save_png(const ek_bitmap* bitmap, const char* path, bool alpha) {
    EK_ASSERT(bitmap != NULL);
    EK_ASSERT(bitmap->pixels != NULL);

    const int w = (int) bitmap->w;
    const int h = (int) bitmap->h;

    // require RGBA non-premultiplied alpha
    //undo_premultiply_image(img);

    stbi_write_png_compression_level = 10;
    stbi_write_force_png_filter = 0;

    if (alpha) {
        stbi_write_png(path, w, h, 4, bitmap->pixels, 4 * w);
    } else {
        const size_t pixels_count = w * h;
        auto* buffer = (uint8_t*) malloc(pixels_count * 3);
        auto* buffer_rgb = buffer;
        const uint8_t* buffer_rgba = (const uint8_t*) bitmap->pixels;

        for (size_t i = 0; i < pixels_count; ++i) {
            buffer_rgb[0] = buffer_rgba[0];
            buffer_rgb[1] = buffer_rgba[1];
            buffer_rgb[2] = buffer_rgba[2];
            buffer_rgba += 4;
            buffer_rgb += 3;
        }

        stbi_write_png(path, w, h, 3, buffer, 3 * w);
        free(buffer);
    }
}

void ek_bitmap_save_jpg(const ek_bitmap* bitmap, const char* path, bool alpha) {
    // require RGBA non-premultiplied alpha
    //undo_premultiply_image(img);
    const int w = (int) bitmap->w;
    const int h = (int) bitmap->h;
    const size_t pixels_count = w * h;
    const uint8_t* buffer_rgba = (uint8_t*) bitmap->pixels;

    if (alpha) {
        auto* buffer_rgb = (uint8_t*) malloc(pixels_count * 3);
        auto* buffer_alpha = (uint8_t*) malloc(pixels_count);

        auto* rgb = buffer_rgb;
        auto* alphaMask = buffer_alpha;
        for (size_t i = 0; i < pixels_count; ++i) {
            rgb[0] = buffer_rgba[0];
            rgb[1] = buffer_rgba[1];
            rgb[2] = buffer_rgba[2];
            alphaMask[0] = buffer_rgba[3];
            buffer_rgba += 4;
            rgb += 3;
            alphaMask += 1;
        }

        char outputPath[1024];
        ek_snprintf(outputPath, sizeof(outputPath), "%s.jpg", path);
        stbi_write_jpg(outputPath, w, h, 3, buffer_rgb, 90);

        ek_snprintf(outputPath, sizeof(outputPath), "%s_a.jpg", path);
        stbi_write_jpg(outputPath, w, h, 1, buffer_alpha, 90);

        free(buffer_rgb);
        free(buffer_alpha);
    } else {
        auto* buffer = (uint8_t*) malloc(pixels_count * 3);
        auto* buffer_rgb = buffer;

        for (size_t i = 0; i < pixels_count; ++i) {
            buffer_rgb[0] = buffer_rgba[0];
            buffer_rgb[1] = buffer_rgba[1];
            buffer_rgb[2] = buffer_rgba[2];
            buffer_rgba += 4;
            buffer_rgb += 3;
        }

        stbi_write_jpg(path, w, h, 3, buffer, 90);

        free(buffer);
    }
}

//
//void undoPremultiplyAlpha(image_t& bitmap) {
//    auto* it = (abgr32_t*) bitmap.data();
//    const auto* end = it + bitmap.width() * bitmap.height();
//
//    while (it < end) {
//        const uint8_t a = it->a;
//        if (a && (a ^ 0xFF)) {
//            const uint8_t half = a / 2;
//            it->r = std::min(255, (it->r * 0xFF + half) / a);
//            it->g = std::min(255, (it->g * 0xFF + half) / a);
//            it->b = std::min(255, (it->b * 0xFF + half) / a);
//        }
//        ++it;
//    }
//}

void save(ImageSet& images, const char* output) {
    pugi::xml_document doc{};
    auto nodeAtlas = doc.append_child("images");
    int idx = 0;
    char path[1024];

    for (auto& resolution: images.resolutions) {
        auto nodeResolution = nodeAtlas.append_child("resolution");
        for (auto& image: resolution.sprites) {
            if (image.bitmap.pixels) {
                auto* bitmap = &image.bitmap;
                // require RGBA non-premultiplied alpha
                ek_bitmap_un_premultiply(bitmap);

                auto nodeSprite = nodeResolution.append_child("image");
//                snprintf(path, 1024, "%s/%d.png", output, idx++);
//                stbi_write_png(path, (int) bitmap.width(), (int) bitmap.height(), 4, bitmap.data(), (int)bitmap.width() * 4);
                snprintf(path, 1024, "%s/%d.bmp", output, idx++);
                stbi_write_bmp(path, (int) bitmap->w, (int) bitmap->h, 4, bitmap->pixels);

                nodeSprite.append_attribute("path").set_value(path);
                nodeSprite.append_attribute("name").set_value(image.name.c_str());
                nodeSprite.append_attribute("x").set_value(image.rc.x);
                nodeSprite.append_attribute("y").set_value(image.rc.y);
                nodeSprite.append_attribute("w").set_value(image.rc.width);
                nodeSprite.append_attribute("h").set_value(image.rc.height);
                nodeSprite.append_attribute("p").set_value(image.padding);
            }
        }
    }
    snprintf(path, 1024, "%s/_images.xml", output);
    doc.save_file(path);
}

}