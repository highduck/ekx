#pragma once

#include <vector>
#include <memory>
#include <ek/graphics/texture.hpp>

namespace ek {

struct DynamicAtlasSprite {
    rect_f texCoords{0, 0, 1, 1};
    graphics::texture_t* texture = nullptr;
};

class DynamicAtlas : private disable_copy_assign_t {
public:
    DynamicAtlas(int pageWidth_, int pageHeight_, bool alphaMap, bool mipmaps);

    ~DynamicAtlas();

    void reset();

    DynamicAtlasSprite addBitmap(int width, int height, const std::vector<uint8_t>& pixels);

    graphics::texture_t* texture_ = nullptr;
    int pageWidth;
    int pageHeight;
private:
    float invWidth;
    float invHeight;
    int bytesPerPixel = 1;
    int padding = 1;
    int x = 0;
    int y = 0;
    int lineHeight = 0;
};

}

