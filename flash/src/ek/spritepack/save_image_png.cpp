#include "save_image_png.hpp"
#include <ek/imaging/drawing.hpp>
#include <stb/stb_image_write.h>

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

// require RGBA non-premultiplied alpha
void save_image_png_stb(const image_t& image, const std::string& path) {
    image_t img{image};
    undo_premultiply_image(img);
    stbi_write_png(path.c_str(),
                   img.width(),
                   img.height(),
                   4,
                   img.data(),
                   4 * static_cast<int>(img.width()));
}

}