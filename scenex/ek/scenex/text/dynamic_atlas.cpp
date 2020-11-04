#include <ek/graphics/gl_debug.hpp>
#include "dynamic_atlas.hpp"

namespace ek {

DynamicAtlas::DynamicAtlas(int pageWidth_, int pageHeight_) :
        texture_{new graphics::texture_t()},
        pageWidth{pageWidth_},
        pageHeight{pageHeight_},
        invWidth{1.0f / pageWidth_},
        invHeight{1.0f / pageHeight_} {

    reset();
}

DynamicAtlas::~DynamicAtlas() {
    delete texture_;
}

void DynamicAtlas::reset() {
    auto emptyData = std::vector<uint8_t>{};
    emptyData.resize(bytesPerPixel * pageWidth * pageHeight, 0);

    glBindTexture(GL_TEXTURE_2D, texture_->handle());

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    graphics::gl::check_error();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, pageWidth, pageHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, emptyData.data());
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pageWidth, pageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, emptyData.data());
    graphics::gl::check_error();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    graphics::gl::check_error();

//    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    graphics::gl::check_error();

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

    glBindTexture(GL_TEXTURE_2D, texture_->handle());

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    graphics::gl::check_error();

    // upload data to texture region
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    graphics::gl::check_error();

    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels.data());
//    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    graphics::gl::check_error();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    graphics::gl::check_error();

    if (height > lineHeight) {
        lineHeight = height;
    }
    x += width + padding;

    return sprite;
}

}