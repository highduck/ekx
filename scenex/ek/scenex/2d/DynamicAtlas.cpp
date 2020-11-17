#include "DynamicAtlas.hpp"
#include <ek/graphics/texture.hpp>

namespace ek {

class DynamicAtlas::Page : private disable_copy_assign_t {
public:
    Page(int width_, int height_, bool alphaMap_, bool mipmaps_) :
            width{width_},
            height{height_},
            invWidth{1.0f / (float) width_},
            invHeight{1.0f / (float) height_},
            bytesPerPixel{alphaMap_ ? 1 : 4},
            alphaMap{alphaMap_},
            mipmaps{mipmaps_} {

        texture = new graphics::texture_t();
        texture->setType(alphaMap ? graphics::texture_type::alpha8 : graphics::texture_type::color32);
        if (mipmaps) {
            texture->setMipMaps(true);
        }
        reset();
    }

    ~Page() {
        delete texture;
    }

    void reset() {
        auto emptyData = std::vector<uint8_t>{};
        emptyData.resize(bytesPerPixel * width * height, 0);

        texture->upload_pixels(width, height, emptyData.data());

        x = padding;
        y = padding;
        lineHeight = 0;
    }

    bool add(int spriteWidth, int spriteHeight, const std::vector<uint8_t>& pixels, DynamicAtlasSprite& sprite) {
        assert(pixels.size() >= spriteWidth * spriteHeight * bytesPerPixel);
        assert(spriteWidth < width && spriteHeight < height);

        int placeX = x;
        int placeY = y;
        int newLineHeight = lineHeight;

        if (placeX + spriteWidth + padding > width) {
            placeX = 0;
            placeY += newLineHeight + padding;
            newLineHeight = 0;
        }
        if (placeY + spriteHeight > height) {
            // no
            return false;
        }

        sprite.texture = texture;
        sprite.texCoords.set(invWidth * placeX, invHeight * placeY, invWidth * spriteWidth, invHeight * spriteHeight);

        texture->updateRect(placeX, placeY, spriteWidth, spriteHeight, pixels.data());

        if (newLineHeight < spriteHeight) {
            newLineHeight = spriteHeight;
        }
        placeX += spriteWidth + padding;

        x = placeX;
        y = placeY;
        lineHeight = newLineHeight;

        return true;
    }

    graphics::texture_t* texture;
    int width;
    int height;
    float invWidth;
    float invHeight;
    int bytesPerPixel = 1;
    int padding = 1;
    int x = 0;
    int y = 0;
    int lineHeight = 0;

    bool alphaMap;
    bool mipmaps;
};

DynamicAtlas::DynamicAtlas(int pageWidth_, int pageHeight_, bool alphaMap_, bool mipmaps_) :
        pageWidth{pageWidth_},
        pageHeight{pageHeight_},
        alphaMap{alphaMap_},
        mipmaps{mipmaps_} {

    pages_.push_back(new Page(pageWidth_, pageHeight_, alphaMap, mipmaps));
}

DynamicAtlas::~DynamicAtlas() {
    for (auto* page : pages_) {
        delete page;
    }
}

DynamicAtlasSprite DynamicAtlas::addBitmap(int width, int height, const std::vector<uint8_t>& pixels) {
    int bpp = alphaMap ? 1 : 4;
    assert(pixels.size() >= width * height * bpp);
    assert(width < pageWidth && height < pageHeight);

    DynamicAtlasSprite sprite;
    for (auto* page : pages_) {
        if (page->add(width, height, pixels, sprite)) {
            return sprite;
        }
    }
    auto* newPage = new Page(pageWidth, pageHeight, alphaMap, mipmaps);
    pages_.push_back(newPage);
    if (!newPage->add(width, height, pixels, sprite)) {
        // how come?
        assert(false);
    }

    return sprite;
}

const graphics::texture_t* DynamicAtlas::getPageTexture(int index) const {
    assert(index < pages_.size() && index >= 0);
    return pages_[index]->texture;
}

}