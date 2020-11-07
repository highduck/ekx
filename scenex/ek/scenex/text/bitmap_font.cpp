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
        BitmapFontData fontData;
        io(fontData);

        load(fontData);
    }
}

void BitmapFont::load(const BitmapFontData& data) {
    units_per_em = data.units_per_em;
    bitmap_size_table = data.sizes;
    for (const auto& g : data.glyphs) {
        for (auto code : g.codes) {
            map[code] = g;
        }
    }
}

bool BitmapFont::getGlyph(uint32_t codepoint, Glyph& outGlyph) {
    auto it = map.find(codepoint);
    if (it != map.end()) {
        const auto& g = it->second;
        outGlyph.advanceWidth = metricsScale * g.advance_width;
        asset_t<sprite_t> spr{g.sprite + '_' + std::to_string(baseFontSize)};
        if (spr) {
            outGlyph.rect = spr->rect * rasterScale;
            outGlyph.texCoord = spr->tex;
            outGlyph.texture = spr->texture.get();
            outGlyph.rotated = spr->rotated;
        } else {
            outGlyph.rect = rect_f{g.box} * metricsScale;
        }
        return true;
    }
    return false;
}

void BitmapFont::setFontSize(float fontSize) {
    metricsScale = fontSize / static_cast<float>(units_per_em);

    int bitmap_size_index = static_cast<int>(bitmap_size_table.size()) - 1;
    while (bitmap_size_index > 0) {
        if (fontSize <= static_cast<float>(bitmap_size_table[bitmap_size_index - 1])) {
            --bitmap_size_index;
        } else {
            break;
        }
    }

    baseFontSize = bitmap_size_table[bitmap_size_index];
    rasterScale = fontSize / static_cast<float>(baseFontSize);
}

}