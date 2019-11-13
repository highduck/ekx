#include "font.hpp"

#include "sprite.hpp"
#include <ek/locator.hpp>
#include <platform/static_resources.hpp>
#include <draw2d/drawer.hpp>
#include <ek/math/bounds_builder.hpp>
#include <ek/logger.hpp>

using namespace ek;

namespace scenex {

font_t::font_t(const font_data_t& data) {
    units_per_em = data.units_per_em;
    bitmap_size_table = data.sizes;
    for (const auto& g : data.glyphs) {
        for (auto code : g.codes) {
            map[code] = g;
        }
    }
}

const font_glyph_t* font_t::get_glyph(uint32_t code) const {
    auto it = map.find(code);
    return it != map.end() ? &it->second : nullptr;
}

void font_t::draw(const std::string& text,
                  float size,
                  const float2& position,
                  argb32_t color,
                  float line_height,
                  float line_spacing) const {

    auto sc = size / static_cast<float>(units_per_em);

    float2 current = position;
    float2 start = position;

    auto& drawer = resolve<drawer_t>();
    drawer.save_color().multiply_color(color);
    // var vertexColor = drawer.calcVertexColorMultiplier(color);

    int bitmap_size_index = static_cast<int>(bitmap_size_table.size()) - 1;
    while (bitmap_size_index > 0) {
        if (size <= static_cast<float>(bitmap_size_table[bitmap_size_index - 1])) {
            --bitmap_size_index;
        } else {
            break;
        }
    }

    int bm_font_size = bitmap_size_table[bitmap_size_index];
    float bitmap_scale = size / bm_font_size;

    for (char code : text) {
        if (code == '\n' || code == '\r') {
            current.x = start.x;
            current.y += line_height + line_spacing;
            continue;
        }

        auto gdata = get_glyph(code);
        if (gdata == nullptr || gdata->sprite.empty()) continue;

        asset_t<sprite_t> spr{
                gdata->sprite + '_' + std::to_string(bm_font_size)
        };

        if (spr) {
            spr->draw(rect_f{
                    bitmap_scale * spr->rect.x + current.x,
                    bitmap_scale * spr->rect.y + current.y,
                    bitmap_scale * spr->rect.width,
                    bitmap_scale * spr->rect.height
            });

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

        current.x += sc * gdata->advance_width;
    }
    drawer.restore_color();
}

float font_t::get_text_segment_width(const std::string& text, float size, int begin, int end) const {
    const float sc = size / units_per_em;
    float x = 0.0f;
    float max = 0.0f;
    for (int i = begin; i < end; ++i) {
        auto c = text[i];
        if (c == '\n' || c == '\r') {
            x = 0.0f;
        }
        auto gdata = get_glyph(text[i]);
        if (gdata == nullptr) continue;
        x += gdata->advance_width * sc;
        if (max < x) {
            max = x;
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
    float em = units_per_em;
    float sc = size / em;
    bounds_builder_2f bounds_builder;
    float x = 0.0f;
    float y = 0.0f;
    if (end < 0) end = text.size();
    for (int i = begin; i < end; ++i) {
        int code = text[i];
        if (code == '\n' || code == '\r') {
            x = 0.0f;
            y += line_height + line_spacing;
        }
        auto gdata = get_glyph(code);
        if (gdata == nullptr) continue;

        // C-BOX:
        // 0 x-min = 0
        // 1 y-min = 0
        // 2 x-max = 625 * 32p / 1000em = 20
        // 3 y-max = 625 * 32p / 1000em = 20

        // x = 0, w = 20
        // y = -h = -20, h = 20

        bounds_builder.add(
                {x + gdata->box[0] * sc, y - gdata->box[3] * sc},
                {x + gdata->box[2] * sc, y - gdata->box[1] * sc}
        );
        x += gdata->advance_width * sc;
    }
    return bounds_builder.rect();
}

rect_f font_t::estimate_text_draw_zone(const std::string& text, float size, int begin, int end, float line_height,
                                       float line_spacing) const {
    float em = units_per_em;
    float sc = size / em;
    bounds_builder_2f bounds_builder;
    float2 cursor{0.0f, 0.0f};
    if (end < 0) {
        end = text.size();
    }
    for (int i = begin; i < end; ++i) {
        int code = text[i];
        if (code == '\n' || code == '\r') {
            cursor.x = 0.0f;
            cursor.y += line_height + line_spacing;
            continue;
        }

        const auto* gdata = get_glyph(code);
        if (gdata == nullptr) {
            continue;
        }

        auto w = gdata->advance_width * sc;
        bounds_builder.add({cursor.x, cursor.y - size, w, size});
        cursor.x += w;
    }
    return bounds_builder.rect();
}

font_t* load_font(const std::string& path) {
    auto buffer = get_resource_content(path);
    font_t* font = nullptr;
    if (buffer.empty()) {
        EK_ERROR << "FONT resource not found: " << path;
    } else {
        input_memory_stream input{buffer.data(), buffer.size()};

        IO io{input};
        font_data_t fontData;
        io(fontData);

        font = new font_t(fontData);
    }
    return font;
}
}
