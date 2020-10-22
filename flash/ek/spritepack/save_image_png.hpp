#pragma once

#include <ek/imaging/image.hpp>
#include <string>

namespace ek {

void save_image_png_stb(const image_t& image, const std::string& path, bool alpha = true);

}

