#include "Font.hpp"

#include <ek/draw2d/drawer.hpp>
#include "FontImplBase.hpp"

namespace ek {

Font::Font(FontImplBase* impl_) :
        impl{impl_} {

}

Font::~Font() {
    delete impl;
}

void Font::draw(const char* text,
                float size,
                const Vec2f& position,
                argb32_t color,
                float line_height,
                float line_spacing) const {

    Vec2f current = position;
    Vec2f start = position;

    draw2d::state.save_color()
            .scaleColor(color);

    const Texture* prevTexture = nullptr;
    Glyph gdata;
    for(; *text; ++text) {
        char code = *text;
        if (code == '\n') {
            current.x = start.x;
            current.y += line_height + line_spacing;
            continue;
        }

        if (impl->getGlyph(code, gdata)) {
            if (gdata.texture) {
                if (prevTexture != gdata.texture) {
                    draw2d::state.setTexture(gdata.texture);
                    prevTexture = gdata.texture;
                }
                draw2d::state.setTextureCoords(gdata.texCoord);
                gdata.rect *= size;
                if (gdata.rotated) {
                    draw2d::quad_rotated(gdata.rect.x + current.x,
                                         gdata.rect.y + current.y,
                                         gdata.rect.width,
                                         gdata.rect.height);

                } else {
                    draw2d::quad(gdata.rect.x + current.x,
                                 gdata.rect.y + current.y,
                                 gdata.rect.width,
                                 gdata.rect.height);
                }
            }

            current.x += size * gdata.advanceWidth;
        }
    }
    draw2d::state.restore_color();
}

float Font::get_text_segment_width(const char* text, float size, int begin, int end) const {
    float x = 0.0f;
    float max = 0.0f;
    Glyph gdata;
    for (int i = begin; i < end; ++i) {
        auto c = text[i];
        if (c == '\n') {
            x = 0.0f;
        }
        if (impl->getGlyph(text[i], gdata)) {
            x += size * gdata.advanceWidth;
            if (max < x) {
                max = x;
            }
        }
    }
    return max;
}

FontType Font::getFontType() const {
    return impl->getFontType();
}

FontImplBase* Font::getImpl() {
    return impl;
}

const FontImplBase* Font::getImpl() const {
    return impl;
}

bool Font::getGlyph(uint32_t codepoint, Glyph& outGlyph) {
    return impl->getGlyph(codepoint, outGlyph) ||
           (fallback && fallback->getGlyph(codepoint, outGlyph));
}

void Font::setBlur(float radius, int iterations, int strengthPower) {
    impl->setBlur(radius, iterations, strengthPower);
    if (fallback) {
        fallback->setBlur(radius, iterations, strengthPower);
    }
}

bool Font::getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) {
    return impl->getGlyphMetrics(codepoint, outGlyph) ||
           (fallback && fallback->getGlyphMetrics(codepoint, outGlyph));
}

}
