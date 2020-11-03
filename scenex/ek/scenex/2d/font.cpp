#include "font.hpp"

#include <ek/util/locator.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/math/bounds_builder.hpp>

namespace ek {

FontImplBase::~FontImplBase() = default;

font_t::font_t(FontImplBase* impl_) :
        impl{impl_} {

}

font_t::~font_t() {
    delete impl;
}

void font_t::draw(const std::string& text,
                  float size,
                  const float2& position,
                  argb32_t color,
                  float line_height,
                  float line_spacing) const {

    auto sizeInfo = impl->getSizeInfo(size);

    float2 current = position;
    float2 start = position;

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

        if (impl->getGlyph(code, sizeInfo, gdata)) {
            if (gdata.texture) {
                if (prevTexture != gdata.texture) {
                    draw2d::state.set_texture(gdata.texture);
                    prevTexture = gdata.texture;
                }
                draw2d::state.set_texture_coords(gdata.texCoord);
                if (gdata.rotated) {
                    draw2d::quad_rotated( gdata.x0 + current.x,
                                          gdata.y0 + current.y,
                                          gdata.x1 - gdata.x0,
                                          gdata.y1 - gdata.y0);

                } else {
                    draw2d::quad(gdata.x0 + current.x,
                                 gdata.y0 + current.y,
                                 gdata.x1 - gdata.x0,
                                 gdata.y1 - gdata.y0);
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

            current.x += gdata.advanceWidth;
        }
    }
    draw2d::state.restore_color();
}

float font_t::get_text_segment_width(const std::string& text, float size, int begin, int end) const {
    auto sizeInfo = impl->getSizeInfo(size);
    float x = 0.0f;
    float max = 0.0f;
    Glyph gdata;
    for (int i = begin; i < end; ++i) {
        auto c = text[i];
        if (c == '\n' || c == '\r') {
            x = 0.0f;
        }
        if (impl->getGlyph(text[i], sizeInfo, gdata)) {
            x += gdata.advanceWidth;
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

rect_f font_t::get_line_bounding_box(const std::string& text, float size, int begin, int end, float line_height,
                                     float line_spacing) const {
    auto sizeInfo = impl->getSizeInfo(size);
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
        if (impl->getGlyph(code, sizeInfo, gdata)) {

            // C-BOX:
            // 0 x-min = 0
            // 1 y-min = 0
            // 2 x-max = 625 * 32p / 1000em = 20
            // 3 y-max = 625 * 32p / 1000em = 20

            // x = 0, w = 20
            // y = -h = -20, h = 20

            bounds_builder.add(
                    {x + gdata.x0, y + gdata.y0},
                    {x + gdata.x1, y + gdata.y1}
            );
            x += gdata.advanceWidth;
        }
    }
    return bounds_builder.rect();
}

rect_f font_t::estimate_text_draw_zone(const std::string& text, float size, int begin, int end, float line_height,
                                       float line_spacing) const {
    auto sizeInfo = impl->getSizeInfo(size);
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

        if (impl->getGlyph(code, sizeInfo, gdata)) {

            auto w = gdata.advanceWidth;
            bounds_builder.add({cursor.x, cursor.y - size, w, size});
            cursor.x += w;
        }
    }
    return bounds_builder.rect();
}

void font_t::debugDrawAtlas() {
    impl->debugDrawAtlas();
}

FontType font_t::getType() const {
    return impl->getType();
}

const FontImplBase* font_t::getImpl() const {
    return impl;
}


}
