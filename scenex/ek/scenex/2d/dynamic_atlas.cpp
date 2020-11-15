#include "dynamic_atlas.hpp"

namespace ek {

DynamicAtlas::DynamicAtlas(int pageWidth_, int pageHeight_, bool alphaMap, bool mipmaps) :
        texture_{new graphics::texture_t()},
        pageWidth{pageWidth_},
        pageHeight{pageHeight_},
        invWidth{1.0f / (float) pageWidth_},
        invHeight{1.0f / (float) pageHeight_} {

    texture_->setType(alphaMap ? graphics::texture_type::alpha8 : graphics::texture_type::color32);
    bytesPerPixel = alphaMap ? 1 : 4;
    if (mipmaps) {
        texture_->setMipMaps(true);
    }
    reset();
}

DynamicAtlas::~DynamicAtlas() {
    delete texture_;
}

void DynamicAtlas::reset() {
    auto emptyData = std::vector<uint8_t>{};
    emptyData.resize(bytesPerPixel * pageWidth * pageHeight, 0);

    texture_->upload_pixels(pageWidth, pageHeight, emptyData.data());

    x = padding;
    y = padding;
    lineHeight = 0;
}

DynamicAtlasSprite DynamicAtlas::addBitmap(int width, int height, const std::vector<uint8_t>& pixels) {
    assert(pixels.size() >= width * height * bytesPerPixel);

    if (x + width + padding > pageWidth) {
        x = 0;
        y += lineHeight + padding;
        lineHeight = 0;
    }
    DynamicAtlasSprite sprite;
    sprite.texture = texture_;
    sprite.texCoords.set(invWidth * x, invHeight * y, invWidth * width, invHeight * height);

    texture_->updateRect(x, y, width, height, pixels.data());

    if (height > lineHeight) {
        lineHeight = height;
    }
    x += width + padding;

    return sprite;
}

}