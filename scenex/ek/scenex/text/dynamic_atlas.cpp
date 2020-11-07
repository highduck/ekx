#include "dynamic_atlas.hpp"

namespace ek {

DynamicAtlas::DynamicAtlas(int pageWidth_, int pageHeight_) :
        texture_{new graphics::texture_t()},
        pageWidth{pageWidth_},
        pageHeight{pageHeight_},
        invWidth{1.0f / (float) pageWidth_},
        invHeight{1.0f / (float) pageHeight_} {

    reset();
}

DynamicAtlas::~DynamicAtlas() {
    delete texture_;
}

void DynamicAtlas::reset() {
    auto emptyData = std::vector<uint8_t>{};
    emptyData.resize(bytesPerPixel * pageWidth * pageHeight, 0);

    texture_->uploadAlpha8(pageWidth, pageHeight, emptyData.data());

    x = padding;
    y = padding;
    lineHeight = 0;
}

DynamicAtlasSprite DynamicAtlas::addBitmap(int width, int height, const std::vector<uint8_t>& pixels) {
    if (x + width + padding > pageWidth) {
        x = padding;
        y += lineHeight;
        lineHeight = 0;
    }
    DynamicAtlasSprite sprite;
    sprite.texture = texture_;
    sprite.texCoords.set(invWidth * x, invHeight * y, invWidth * width, invHeight * height);

    texture_->uploadSubAlpha8(x, y, width, height, pixels.data());

    if (height > lineHeight) {
        lineHeight = height;
    }
    x += width + padding;

    return sprite;
}

}