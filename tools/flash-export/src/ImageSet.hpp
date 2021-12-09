#pragma once

#include <ek/math/Rect.hpp>
#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>
#include <ek/imaging/image.hpp>

namespace ek {

struct SpriteData {

    String name;

    // physical rect
    Rect2f rc;

    // rect in source image
    Rect2i source;

    uint8_t padding = 1;

    // reference image;
    image_t* image = nullptr;

    // TODO:
    bool trim = false;
};

struct Resolution {
    int index = 0;
    float scale = 1.0f;
    Array<SpriteData> sprites;
};

struct ImageSet {
    String name;
    Array<Resolution> resolutions;
};

void saveImagePNG(const image_t& image, const char* path, bool alpha = true);

void saveImageJPG(const image_t& image, const char* path, bool alpha = true);

void save(ImageSet& images, const char* output);

}