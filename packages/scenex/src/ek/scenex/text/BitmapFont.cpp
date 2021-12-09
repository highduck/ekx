#include "BitmapFont.hpp"
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/graphics/graphics.hpp>

namespace ek {

BitmapFont::BitmapFont() :
        FontImplBase(FontType::Bitmap) {

}

BitmapFont::~BitmapFont() = default;

void BitmapFont::load(const uint8_t* buffer, size_t length) {
    if (length > 0) {
        input_memory_stream input{buffer, length};

        IO io{input};
        BMFont fontData;
        io(fontData);

        load(fontData);
    }
}

void BitmapFont::load(BMFont& data) {
    unitsPerEM = data.unitsPerEM;
    baseFontSize = data.fontSize;
    ascender = float(data.ascender) / unitsPerEM;
    descender = float(data.descender) / unitsPerEM;
    lineHeightMultiplier = float(data.lineHeight) / unitsPerEM;
    glyphs = std::move(data.glyphs);
    for (const auto& e : data.codepoints) {
        map.set(e.key, glyphs.begin() + e.value);
    }

    ready_ = loaded_ = true;
}

bool BitmapFont::getGlyph(uint32_t codepoint, Glyph& outGlyph) {
    const auto* g = map.get(codepoint, nullptr);
    if (g) {
        outGlyph.advanceWidth = static_cast<float>(g->advanceWidth) / unitsPerEM;
        outGlyph.lineHeight = lineHeightMultiplier;
        Res<Sprite> spr{g->sprite};
        if (spr) {
            outGlyph.rect = spr->rect / baseFontSize;
            outGlyph.texCoord = spr->tex;
            outGlyph.texture = spr->texture.get();
            outGlyph.rotated = spr->rotated;
        } else {
            outGlyph.rect = g->box / unitsPerEM;
        }
        outGlyph.source = this;
        return true;
    }
    return false;
}

bool BitmapFont::getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) {
    const auto* g = map.get(codepoint, nullptr);
    if (g) {
        outGlyph.advanceWidth = static_cast<float>(g->advanceWidth) / unitsPerEM;
        outGlyph.lineHeight = lineHeightMultiplier;
        outGlyph.ascender = ascender;
        outGlyph.descender = descender;
        outGlyph.rect = g->box / unitsPerEM;
        outGlyph.source = this;
        return true;
    }
    return false;
}

float BitmapFont::getKerning(uint32_t codepoint1, uint32_t codepoint2) {
    return 0.0f;
}

}