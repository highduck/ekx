#pragma once

//#include <ek/ds/Array.hpp>
#include <ek/ds/PodArray.hpp>
#include <ek/math.h>
#include <cstring>
#include <sokol_gfx.h>
#include <ek/hash.h>
#include <ek/rr.h>

namespace ek {

struct DynamicAtlasSprite {
    rect_t texCoords = rect_01();
    sg_image image = {0};
};

class DynamicAtlas {
public:
    class Page;

    DynamicAtlas(int pageWidth_, int pageHeight_, bool alphaMap_, bool mipmaps_);

    ~DynamicAtlas();

    DynamicAtlasSprite addBitmap(int width, int height, const uint8_t* pixels, size_t pixelsSize);

    [[nodiscard]] sg_image get_page_image(int index) const;

    void reset();

    static int estimateBetterSize(float scaleFactor, unsigned baseSize, unsigned maxSize);

    void invalidate();

public:
    PodArray<Page*> pages_;
    int pageWidth;
    int pageHeight;
    bool alphaMap;
    bool mipmaps;

    // after reset we increase version, so clients could check if cache maps should be cleared
    unsigned version = 0;
private:
};

}

typedef ek::DynamicAtlas* dynamic_atlas_ptr;

struct res_dynamic_atlas {
    string_hash_t names[16];
    dynamic_atlas_ptr data[16];
    rr_man_t rr;
};

extern struct res_dynamic_atlas res_dynamic_atlas;

void setup_res_dynamic_atlas(void);
void update_res_dynamic_atlas(void);

#define R_DYNAMIC_ATLAS(name) REF_NAME(res_dynamic_atlas, name)


