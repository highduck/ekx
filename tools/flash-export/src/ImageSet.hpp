#pragma once

#include <ek/math/Rect.hpp>
#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>
#include <ek/bitmap.h>

namespace ek {

struct SpriteData {

    String name;

    // physical rect
    Rect2f rc;

    // rect in source image
    Rect2i source;

    uint8_t padding = 1;

    // reference image;
    ek_bitmap bitmap{0, 0, nullptr};

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

void ek_bitmap_save_png(const ek_bitmap* bitmap, const char* path, bool alpha);

void ek_bitmap_save_jpg(const ek_bitmap* bitmap, const char* path, bool alpha);

void save(ImageSet& bitmaps, const char* output);

}