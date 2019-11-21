#include "save_image_png.hpp"
#include <ek/imaging/drawing.hpp>
#include "../../../../external/stb/stb_image_write.h"

namespace ek {
//
//void save_image_png_cairo(const image_t& image, const std::string& path) {
//    auto surface = cairo_image_surface_create_for_data(
//            const_cast<uint8_t*>(image.data()),
//            CAIRO_FORMAT_ARGB32,
//            static_cast<int>(image.width()),
//            static_cast<int>(image.height()),
//            static_cast<int>(image.width() * 4)
//    );
//    cairo_surface_write_to_png(surface, path.c_str());
//    cairo_surface_destroy(surface);
//}

void save_image_png_stb(const image_t& image, const std::string& path, bool alpha) {
    image_t img{image};
    // require RGBA non-premultiplied alpha
    undo_premultiply_image(img);

    if (alpha) {
        stbi_write_png(path.c_str(),
                       img.width(),
                       img.height(),
                       4,
                       img.data(),
                       4 * static_cast<int>(img.width()));
    } else {

        size_t pixels_count = img.width() * img.height();
        auto* buffer = new uint8_t[pixels_count * 3];
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

        delete[] buffer;
    }
}

}