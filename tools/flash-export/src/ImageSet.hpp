#pragma once

#include <ek/math.h>
#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>
#include <ek/bitmap.h>

namespace ek {

struct SpriteData {

    String name;

    // physical rect
    rect_t rc;

    // rect in source image
    recti_t source;

    // reference image;
    ek_bitmap bitmap{0, 0, nullptr};

    uint8_t padding = 1;

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

// TODO: move to ek/bitmap.h
void ek_bitmap_save_png(const ek_bitmap* bitmap, const char* path, bool alpha);
// TODO: move to ek/bitmap.h
void ek_bitmap_save_jpg(const ek_bitmap* bitmap, const char* path, bool alpha);

void save(ImageSet& bitmaps, const char* output);

}