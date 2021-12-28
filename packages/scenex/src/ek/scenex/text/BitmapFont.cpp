#include "BitmapFont.hpp"
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/util/Res.hpp>

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

    for (int i = 0; i < data.codepoints.size();) {
        const uint32_t codepoint = data.codepoints[i++];
        const uint32_t glyph_id = data.codepoints[i++];
        map.set(codepoint, glyphs.begin() + glyph_id);
    }

    ready_ = loaded_ = true;
}

bool BitmapFont::getGlyph(uint32_t codepoint, Glyph& outGlyph) {
    const auto* g = map.get(codepoint, nullptr);
    if (g) {
        outGlyph.advanceWidth = static_cast<float>(g->advanceWidth) / unitsPerEM;
        outGlyph.lineHeight = lineHeightMultiplier;
        Res<Sprite> spr{g->sprite.c_str()};
        if (spr) {
            outGlyph.rect = spr->rect / baseFontSize;
            outGlyph.texCoord = spr->tex;
            outGlyph.image = ek_image_reg_get(spr->image_id);
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
    (void)codepoint1;
    (void)codepoint2;
    return 0.0f;
}

}