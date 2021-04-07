#include "DynamicAtlas.hpp"
#include <ek/graphics/graphics.hpp>
#include <ek/math/common.hpp>

namespace ek {

//void copy_pixels_normal(image_t& dest, int2 dest_position,
//                        const image_t& src, const rect_i& src_rect) {
//    rect_i dest_rc = {dest_position, src_rect.size};
//    rect_i src_rc = src_rect;
//    clip_rects(src.bounds<int>(), dest.bounds<int>(),
//               src_rc, dest_rc);
//
//    for (int32_t y = 0; y < src_rc.height; ++y) {
//        for (int32_t x = 0; x < src_rc.width; ++x) {
//            const auto pixel = get_pixel_unsafe(src, int2{src_rc.x + x, src_rc.y + y});
//            set_pixel_unsafe(dest, {dest_rc.x + x, dest_rc.y + y}, pixel);
//        }
//    }
//}

class DynamicAtlas::Page : private disable_copy_assign_t {
public:
    Page(int width_, int height_, bool alphaMap_, bool mipmaps_) :
            width{width_},
            height{height_},
            invWidth{1.0f / (float) width_},
            invHeight{1.0f / (float) height_},
            bytesPerPixel{alphaMap_ ? 1 : 4},
            padding{1},
            x{1},
            y{1},
            lineHeight{0},
            alphaMap{alphaMap_},
            mipmaps{mipmaps_},
            data(width_ * height_ * (alphaMap_ ? 1 : 4), 0u),
            dirtyRect{0, 0, width_, height_},
            dirty{true} {

        sg_image_desc desc{
                .type = SG_IMAGETYPE_2D,
                .width = width,
                .height = height,
                .usage = SG_USAGE_DYNAMIC,
                .pixel_format = alphaMap ? SG_PIXELFORMAT_R8 : SG_PIXELFORMAT_RGBA8,
                .min_filter = SG_FILTER_LINEAR,
                .mag_filter = SG_FILTER_LINEAR,
                .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
                .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
        };
        texture = new graphics::Texture(desc);
    }

    ~Page() {
        delete texture;
    }

    void reset() {
        //data.assign(data.size(), 0);
        memset(data.data(), 0u, data.size());
        dirtyRect.set(0, 0, width, height);
        dirty = true;
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

        //texture->updateRect(placeX, placeY, spriteWidth, spriteHeight, pixels.data());

        {
            auto srcStride = bytesPerPixel * spriteWidth;
            auto destStride = bytesPerPixel * width;
            for (int cy = 0; cy < spriteHeight; ++cy) {
                memcpy(data.data() + placeX + (placeY + cy) * destStride, pixels.data() + cy * srcStride, srcStride);
            }
            if (!dirty) {
                dirtyRect.set(placeX, placeY, spriteWidth, spriteHeight);
            } else {
                dirtyRect = combine(dirtyRect, {placeX, placeY, spriteWidth, spriteHeight});
            }
            dirty = true;
        }

        if (newLineHeight < spriteHeight) {
            newLineHeight = spriteHeight;
        }
        placeX += spriteWidth + padding;

        x = placeX;
        y = placeY;
        lineHeight = newLineHeight;

        return true;
    }

    void invalidate() {
        if (dirty) {
            texture->update(data.data(), data.size());
            dirty = false;
        }
    }

    graphics::Texture* texture;
    int width;
    int height;
    float invWidth;
    float invHeight;
    int bytesPerPixel = 1;
    int padding = 1;
    int x = 0;
    int y = 0;
    int lineHeight = 0;

    std::vector<uint8_t> data;
    rect_i dirtyRect;
    bool dirty;


    bool alphaMap;
    bool mipmaps;
};

DynamicAtlas::DynamicAtlas(int pageWidth_, int pageHeight_, bool alphaMap_, bool mipmaps_) :
        pageWidth{pageWidth_},
        pageHeight{pageHeight_},
        alphaMap{alphaMap_},
        mipmaps{mipmaps_} {

    auto* page = new Page(pageWidth_, pageHeight_, alphaMap, mipmaps);
    pages_.push_back(page);
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
    newPage->reset();
    pages_.push_back(newPage);
    if (!newPage->add(width, height, pixels, sprite)) {
        // how come?
        assert(false);
    }

    return sprite;
}

const graphics::Texture* DynamicAtlas::getPageTexture(int index) const {
    assert(index < pages_.size() && index >= 0);
    return pages_[index]->texture;
}

void DynamicAtlas::reset() {
    for (auto& page : pages_) {
        page->reset();
    }
    ++version;
}

int DynamicAtlas::estimateBetterSize(float scaleFactor, unsigned baseSize, unsigned maxSize) {
    auto scaledSize = static_cast<unsigned>(ceilf(static_cast<float>(baseSize) * scaleFactor));
    auto potSize = math::nextPowerOf2(scaledSize);
    return std::min(potSize, maxSize);
}

void DynamicAtlas::invalidate() {
    for (auto& page : pages_) {
        page->invalidate();
    }
}

}