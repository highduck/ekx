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
    DynamicAtlas(int pageWidth_, int pageHeight_);

    ~DynamicAtlas();

    void reset();

    DynamicAtlasSprite addBitmap(int width, int height, const std::vector<uint8_t>& pixels);

    int fontSize;
    int pageWidth;
    int pageHeight;
    graphics::texture_t* texture_ = nullptr;
private:
//    std::vector<DynamicAtlasSprite> sprites;
    float invWidth = 1.0f / 2048.0f;
    float invHeight = 1.0f / 2048.0f;
    int bytesPerPixel = 1;
    int padding = 1;
    int x = 0;
    int y = 0;
    int lineHeight = 0;
};

}

