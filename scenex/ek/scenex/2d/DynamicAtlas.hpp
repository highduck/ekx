#pragma once

#include <vector>
#include <ek/util/common_macro.hpp>
#include <ek/math/box.hpp>

namespace ek {

namespace graphics {
class texture_t;
}

struct DynamicAtlasSprite {
    rect_f texCoords{0, 0, 1, 1};
    graphics::texture_t* texture = nullptr;
};

class DynamicAtlas : private disable_copy_assign_t {
public:
    class Page;

    DynamicAtlas(int pageWidth_, int pageHeight_, bool alphaMap_, bool mipmaps_);

    ~DynamicAtlas();

    DynamicAtlasSprite addBitmap(int width, int height, const std::vector<uint8_t>& pixels);

    [[nodiscard]] const graphics::texture_t* getPageTexture(int index) const;

public:
    std::vector<Page*> pages_;
    int pageWidth;
    int pageHeight;
    bool alphaMap;
    bool mipmaps;
private:
};

}

