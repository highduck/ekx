#include "Font.hpp"

#include <ek/canvas.h>
#include "FontImplBase.hpp"

struct res_font res_font;

void setup_res_font(void) {
    struct res_font* R = &res_font;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

namespace ek {

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
    for (; *text; ++text) {
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

bool Font::getGlyph(uint32_t codepoint, Glyph& outGlyph) {
    if (impl->getGlyph(codepoint, outGlyph)) {
        return true;
    }
    if (fallback) {
        auto* fb = &REF_RESOLVE(res_font, fallback);
        if (fb->getGlyph(codepoint, outGlyph)) {
            return true;
        }
    }
    return false;
}

void Font::setBlur(float radius, int iterations, int strengthPower) {
    impl->setBlur(radius, iterations, strengthPower);
    if (fallback) {
        auto* fb = &REF_RESOLVE(res_font, fallback);
        fb->setBlur(radius, iterations, strengthPower);
    }
}

bool Font::getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) {
    if (impl->getGlyphMetrics(codepoint, outGlyph)) {
        return true;
    }
    if (fallback) {
        auto* fb = &REF_RESOLVE(res_font, fallback);
        if (fb->getGlyphMetrics(codepoint, outGlyph)) {
            return true;
        }
    }
    return false;
}

void Font::setFallbackFont(string_hash_t fallbackFont) {
    fallback = R_FONT(fallbackFont);
}

}
