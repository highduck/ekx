#pragma once

#include "Sprite.hpp"

#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>
#include <ek/rr.h>
#include <ek/texture_loader.h>

namespace ek {

class Atlas {
public:

    Atlas();

    ~Atlas();

    void clear();

    Array<R(sprite_t)> sprites;
    Array<R(sg_image)> pages;

    // loading routine
    Array<ek_texture_loader*> loaders;
    String base_path;
    uint32_t formatMask = 1;

    [[nodiscard]] int getLoadingImagesCount() const;
    int pollLoading();

    void load(const char* path, float scaleFactor);
};

}

typedef ek::Atlas* atlas_ptr;

struct res_atlas {
    string_hash_t names[32];
    atlas_ptr data[32];
    rr_man_t rr;
};

extern struct res_atlas res_atlas;

void setup_res_atlas(void);
void update_res_atlas(void);

#define R_ATLAS(name) REF_NAME(res_atlas, name)
