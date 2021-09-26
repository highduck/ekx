#pragma once

#include <ek/math/box.hpp>
#include <string>
#include <vector>
#include <ek/imaging/image.hpp>

namespace ek {

struct SpriteData {

    std::string name;

    // physical rect
    rect_f rc;

    // rect in source image
    rect_i source;

    uint8_t padding = 1;

    // reference image;
    image_t* image = nullptr;

    // TODO:
    bool trim = false;
};

struct Resolution {
    int index = 0;
    float scale = 1.0f;
    std::vector<SpriteData> sprites;
};

struct ImageSet {
    std::string name;
    std::vector<Resolution> resolutions;
};

void saveImagePNG(const image_t& image, const std::string& path, bool alpha = true);
void saveImageJPG(const image_t& image, const std::string& path, bool alpha = true);
void save(ImageSet& images, const char* output);

}