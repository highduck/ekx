#pragma once

#include "sprpk_image.h"
#include <ek/hash.h>

typedef struct {
    float scale;
    uint16_t max_width;
    uint16_t max_height;
} resolution_config_t;

typedef enum sprite_flag_t {
    SPRITE_FLAG_ROTATED = 1u,
    SPRITE_FLAG_PACKED = 2u,
    SPRITE_FLAG_TRIM = 4u
} sprite_flag_t;

typedef struct sprite_data_t {

    string_hash_t name;

    // physical rect
    rect_t rc;

    // coords in atlas image
    rect_t uv;

    // flags in atlas image
    uint8_t flags;

    // rect in source image
    irect_t source;

    uint8_t padding;

    // reference image;
    bitmap_t bitmap;
} sprite_data_t;

typedef struct atlas_page_t {
    uint16_t w;
    uint16_t h;
    sprite_data_t* sprites;
    uint32_t sprites_num;
    bitmap_t bitmap;
} atlas_page_t;

typedef struct atlas_res_t {
    // input
    resolution_config_t resolution;
    sprite_data_t* sprites;
    uint32_t sprites_num;

    // output
    atlas_page_t pages[32];
    uint32_t pages_num;
} atlas_res_t;

void free_atlas_resolutions(atlas_res_t* resolutions, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i) {
        for (uint32_t j = 0; j < resolutions[i].pages_num; ++j) {
            bitmap_free(&resolutions[i].pages[j].bitmap);
            free(resolutions[i].pages[j].sprites);
        }
        resolutions[i].pages_num = 0;
    }
}

void packAndSaveMultiThreaded(atlas_res_t* resolutions, uint32_t resolutions_num, const char* atlas_name, const char* output_path);
