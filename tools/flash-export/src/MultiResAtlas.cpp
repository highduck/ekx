#include "MultiResAtlas.hpp"
#include <pugixml.hpp>
#include <ek/debug.hpp>
#include <ek/util/Path.hpp>
#include <ek/math/max_rects.hpp>
#include <ek/imaging/drawing.hpp>
#include <ek/assert.hpp>

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

#pragma clang diagnostic pop

#endif

namespace ek {

void MultiResAtlasSettings::readFromXML(const pugi::xml_node& node) {
    name = node.attribute("name").as_string();
    for (auto& resolution_node: node.children("resolution")) {
        Resolution res{};
        res.scale = resolution_node.attribute("scale").as_float(res.scale);
        res.max_size.x = resolution_node.attribute("max_width").as_int(res.max_size.x);
        res.max_size.y = resolution_node.attribute("max_height").as_int(res.max_size.y);
        resolutions.push_back(res);
    }
}

// TODO: atlas
// ++page_index;
// page.image_path = atlas.name + get_atlas_suffix(atlas.scale, page_index) + ".png";

/*** Save Image ***/

void saveImagePNG(const image_t& image, const std::string& path, bool alpha) {
    image_t img{image};
    // require RGBA non-premultiplied alpha
    //undo_premultiply_image(img);

    stbi_write_png_compression_level = 10;
    stbi_write_force_png_filter = 0;

    if (alpha) {
        stbi_write_png(path.c_str(),
                       img.width(),
                       img.height(),
                       4,
                       img.data(),
                       4 * static_cast<int>(img.width()));
    } else {

        size_t pixels_count = img.width() * img.height();
        auto* buffer = (uint8_t*) malloc(pixels_count * 3);
        auto* buffer_rgb = buffer;
        auto* buffer_rgba = img.data();

        for (size_t i = 0; i < pixels_count; ++i) {
            buffer_rgb[0] = buffer_rgba[0];
            buffer_rgb[1] = buffer_rgba[1];
            buffer_rgb[2] = buffer_rgba[2];
            buffer_rgba += 4;
            buffer_rgb += 3;
        }

        stbi_write_png(path.c_str(),
                       img.width(),
                       img.height(),
                       3,
                       buffer,
                       3 * static_cast<int>(img.width()));

        free(buffer);
    }
}

void saveImageJPG(const image_t& image, const std::string& path, bool alpha) {
    image_t img{image};
    // require RGBA non-premultiplied alpha
    //undo_premultiply_image(img);

    if (alpha) {
        size_t pixels_count = img.width() * img.height();
        auto* buffer_rgb = (uint8_t*) malloc(pixels_count * 3);
        auto* buffer_alpha = (uint8_t*) malloc(pixels_count);
        auto* buffer_rgba = img.data();

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

        stbi_write_jpg((path + ".jpg").c_str(),
                       img.width(),
                       img.height(),
                       3,
                       buffer_rgb,
                       90);

        stbi_write_jpg((path + "a.jpg").c_str(),
                       img.width(),
                       img.height(),
                       1,
                       buffer_alpha,
                       90);

        free(buffer_rgb);
        free(buffer_alpha);
    } else {

        size_t pixels_count = img.width() * img.height();
        auto* buffer = (uint8_t*) malloc(pixels_count * 3);
        auto* buffer_rgb = buffer;
        auto* buffer_rgba = img.data();

        for (size_t i = 0; i < pixels_count; ++i) {
            buffer_rgb[0] = buffer_rgba[0];
            buffer_rgb[1] = buffer_rgba[1];
            buffer_rgb[2] = buffer_rgba[2];
            buffer_rgba += 4;
            buffer_rgb += 3;
        }

        stbi_write_jpg(path.c_str(),
                       img.width(),
                       img.height(),
                       3,
                       buffer,
                       3 * static_cast<int>(img.width()));

        free(buffer);
    }
}

}