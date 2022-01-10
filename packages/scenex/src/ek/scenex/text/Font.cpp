#include "Font.hpp"

#include <ek/canvas.h>
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
                vec2_t position,
                color_t color,
                float line_height,
                float line_spacing) const {

    vec2_t current = position;
    vec2_t start = position;

    canvas_save_color();
    canvas_scale_color(color);

    uint32_t prev_image_id = SG_INVALID_ID;
    Glyph gdata;
    for(; *text; ++text) {
        char code = *text;
        if (code == '\n') {
            current.x = start.x;
            current.y += line_height + line_spacing;
            continue;
        }

        if (impl->getGlyph(code, gdata)) {
            if (gdata.image.id) {
                if (prev_image_id != gdata.image.id) {
                    canvas_set_image(gdata.image);
                    prev_image_id = gdata.image.id;
                }
                canvas_set_image_rect(gdata.texCoord);
                gdata.rect = rect_scale_f(gdata.rect, size);
                if (gdata.rotated) {
                    canvas_quad_rotated(gdata.rect.x + current.x,
                                         gdata.rect.y + current.y,
                                         gdata.rect.w,
                                         gdata.rect.h);

                } else {
                    canvas_quad(gdata.rect.x + current.x,
                                 gdata.rect.y + current.y,
                                 gdata.rect.w,
                                 gdata.rect.h);
                }
            }

            current.x += size * gdata.advanceWidth;
        }
    }
    canvas_restore_color();
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
