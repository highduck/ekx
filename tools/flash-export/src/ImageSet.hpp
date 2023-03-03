#pragma once

#include <ek/math.h>
#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>
#include <ek/bitmap.h>

//namespace {

struct SpriteData {

    ek::String name;

    // physical rect
    rect_t rc = {};

    // rect in source image
    irect_t source = {};

    // reference image;
    bitmap_t bitmap{0, 0, nullptr};

    uint8_t padding = 1;

    // TODO:
    bool trim = false;
};

struct Resolution {
    int index = 0;
    float scale = 1.0f;
    ek::Array<SpriteData> sprites;
};

struct ImageSet {
    ek::String name;
    ek::Array<Resolution> resolutions;
};

// TODO: move to ek/bitmap.h
void ek_bitmap_save_png(const bitmap_t* bitmap, const char* path, bool alpha);

// TODO: move to ek/bitmap.h
void ek_bitmap_save_jpg(const bitmap_t* bitmap, const char* path, bool alpha);

void save(ImageSet* bitmaps, const char* output);

//}