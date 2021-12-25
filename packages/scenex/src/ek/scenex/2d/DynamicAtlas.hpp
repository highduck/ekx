#pragma once

#include <ek/ds/Array.hpp>
#include <ek/util/NoCopyAssign.hpp>
#include <ek/util/Type.hpp>
#include <ek/math/Rect.hpp>
#include <cstring>
#include <sokol_gfx.h>

namespace ek {

struct DynamicAtlasSprite {
    Rect2f texCoords{0, 0, 1, 1};
    sg_image image = {0};
};

class DynamicAtlas : private NoCopyAssign {
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
    Array<Page*> pages_;
    int pageWidth;
    int pageHeight;
    bool alphaMap;
    bool mipmaps;

    // after reset we increase version, so clients could check if cache maps should be cleared
    unsigned version = 0;
private:
};

EK_DECLARE_TYPE(DynamicAtlas);
EK_TYPE_INDEX(DynamicAtlas, 6);

}

