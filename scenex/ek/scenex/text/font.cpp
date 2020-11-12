#include "font.hpp"

#include <ek/util/locator.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/math/bounds_builder.hpp>
#include "font_base.hpp"

namespace ek {

Font::Font(FontImplBase* impl_) :
        impl{impl_} {

}

Font::~Font() {
    delete impl;
}

void Font::draw(const std::string& text,
                float size,
                const float2& position,
                argb32_t color,
                float line_height,
                float line_spacing) const {

    float2 current = position;
    float2 start = position;

//    float lineHeight = 0.0f;
    draw2d::state.save_color()
            .multiply_color(color);
    // var vertexColor = drawer.calcVertexColorMultiplier(color);

    const graphics::texture_t* prevTexture = nullptr;
    Glyph gdata;
    for (char code : text) {
        if (code == '\n' || code == '\r') {
            current.x = start.x;
            current.y += line_height + line_spacing;
            continue;
        }

        if (impl->getGlyph(code, gdata)) {
            if (gdata.texture) {
                if (prevTexture != gdata.texture) {
                    draw2d::state.set_texture(gdata.texture);
                    prevTexture = gdata.texture;
                }
                draw2d::state.set_texture_coords(gdata.texCoord);
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

                // SPRITE:
                // x = 0
                // y = -10
                // w = 10
                // h = 10

                // CBOX:
                // 0 x-min = 0
                // 1 y-min = 0
                // 2 x-max = 625 * 32p / 1000em = 20
                // 3 y-max = 625 * 32p / 1000em = 20

                // x = 0, w = 20
                // y = -h = -20, h = 20

            }

            current.x += size * gdata.advanceWidth;
        }
    }
    draw2d::state.restore_color();
}

float Font::get_text_segment_width(const std::string& text, float size, int begin, int end) const {
    float x = 0.0f;
    float max = 0.0f;
    Glyph gdata;
    for (int i = begin; i < end; ++i) {
        auto c = text[i];
        if (c == '\n' || c == '\r') {
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

//rect_f Font::getBoundingBox(const std::string& text, float size, int begin, int end) const {
//    float em = unitsPerEm;
//    float sc = size / em;
//    MinMax2 minMax;
//    float x = 0.0f;
//    float y = 0.0f;
//    for (int i = begin; i < end; ++i) {
//        int code = text[i];
//        auto gdata = getGlyphData(code);
//        if (gdata == nullptr) continue;
//        minMax.add(
//                float2{x + gdata->box[0] * sc, y - gdata->box[3] * sc},
//                float2{x + gdata->box[2] * sc, y - gdata->box[1] * sc}
//        );
//
//        x += gdata->advanceWidth * sc;
//    }
//    return minMax.rect();
//}

rect_f Font::get_line_bounding_box(const std::string& text, float size, int begin, int end, float line_height,
                                   float line_spacing) const {
    bounds_builder_2f bounds_builder;
    float x = 0.0f;
    float y = 0.0f;
    if (end < 0) {
        end = text.size();
    }
    Glyph gdata;
    for (int i = begin; i < end; ++i) {
        int code = text[i];
        if (code == '\n' || code == '\r') {
            x = 0.0f;
            y += line_height + line_spacing;
        }
        if (impl->getGlyph(code, gdata)) {
            gdata.rect *= size;
            bounds_builder.add(
                    {x + gdata.rect.x, y + gdata.rect.y},
                    {x + gdata.rect.right(), y + gdata.rect.bottom()}
            );
            x += size * gdata.advanceWidth;
        }
    }
    return bounds_builder.rect();
}

rect_f Font::estimate_text_draw_zone(const std::string& text, float size, int begin, int end, float line_height,
                                     float line_spacing) const {
    bounds_builder_2f bounds_builder;
    float2 cursor{0.0f, 0.0f};
    if (end < 0) {
        end = text.size();
    }
    Glyph gdata;
    for (int i = begin; i < end; ++i) {
        int code = text[i];
        if (code == '\n' || code == '\r') {
            cursor.x = 0.0f;
            cursor.y += line_height + line_spacing;
            continue;
        }

        if (impl->getGlyph(code, gdata)) {
            auto w = size * gdata.advanceWidth;
            bounds_builder.add({cursor.x, cursor.y - size, w, size});
            cursor.x += w;
        }
    }
    return bounds_builder.rect();
}

void Font::debugDrawAtlas(float x, float y) {
    impl->debugDrawAtlas(x, y);
    if(fallback){
        fallback->debugDrawAtlas(x, y + 512);
    }
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

float Font::getKerning(uint32_t codepoint1, uint32_t codepoint2) {
    return impl->getKerning(codepoint1, codepoint2) ||
           (fallback && fallback->getKerning(codepoint1, codepoint2));
}

}
