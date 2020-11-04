#include "bitmap_font.hpp"
#include <ek/scenex/2d/sprite.hpp>
#include <ek/graphics/texture.hpp>

namespace ek {

BitmapFont::BitmapFont() :
        FontImplBase(FontType::Bitmap) {

}

BitmapFont::~BitmapFont() = default;

void BitmapFont::load(const std::vector<uint8_t>& buffer) {
    if (!buffer.empty()) {
        input_memory_stream input{buffer.data(), buffer.size()};

        IO io{input};
        font_data_t fontData;
        io(fontData);

        load(fontData);
    }
}

void BitmapFont::load(const font_data_t& data) {
    units_per_em = data.units_per_em;
    bitmap_size_table = data.sizes;
    for (const auto& g : data.glyphs) {
        for (auto code : g.codes) {
            map[code] = g;
        }
    }
}

bool BitmapFont::getGlyph(uint32_t codepoint, const FontSizeInfo& size, Glyph& outGlyph) {
    auto it = map.find(codepoint);
    if (it != map.end()) {
        const auto& g = it->second;
        outGlyph.advanceWidth = size.metricsScale * g.advance_width;
        asset_t<sprite_t> spr{g.sprite + '_' + std::to_string(size.baseFontSize)};
        if (spr) {
            outGlyph.texCoord = spr->tex;
            outGlyph.texture = spr->texture.get();
            outGlyph.rotated = spr->rotated;

            outGlyph.x0 = spr->rect.x * size.rasterScale;
            outGlyph.y0 = spr->rect.y * size.rasterScale;
            outGlyph.x1 = spr->rect.right() * size.rasterScale;
            outGlyph.y1 = spr->rect.bottom() * size.rasterScale;
        } else {
            outGlyph.x0 = g.box[0] * size.metricsScale;
            outGlyph.y0 = -g.box[3] * size.metricsScale;
            outGlyph.x1 = g.box[2] * size.metricsScale;
            outGlyph.y1 = -g.box[1] * size.metricsScale;
        }

        return true;
    }
    return false;
}

FontSizeInfo BitmapFont::getSizeInfo(float size) {
    FontSizeInfo info;
    info.metricsScale = size / static_cast<float>(units_per_em);

    int bitmap_size_index = static_cast<int>(bitmap_size_table.size()) - 1;
    while (bitmap_size_index > 0) {
        if (size <= static_cast<float>(bitmap_size_table[bitmap_size_index - 1])) {
            --bitmap_size_index;
        } else {
            break;
        }
    }

    info.baseFontSize = bitmap_size_table[bitmap_size_index];
    info.rasterScale = size / static_cast<float>(info.baseFontSize);
    return info;
}

}