#include "BitmapFont.hpp"
#include <ek/scenex/2d/Sprite.hpp>


namespace ek {

BitmapFont::BitmapFont() :
        FontImplBase(FontType::Bitmap) {

}

BitmapFont::~BitmapFont() = default;

void BitmapFont::load(const uint8_t* buffer, size_t length) {
    if (!length) {
        EK_ASSERT("can't load bmfont, buffer is empty");
        return;
    }
    bmfont_file_map(buffer, &file);
    lineHeightMultiplier = file.header->line_height_multiplier;
    for (uint32_t i = 0, end = file.header->codepoints_num * 2; i < end;) {
        const uint32_t codepoint = file.codepoints[i++];
        const uint32_t glyph_id = file.codepoints[i++];
        map.set(codepoint, glyph_id);
    }

    ready_ = loaded_ = true;
}

bool BitmapFont::getGlyph(uint32_t codepoint, Glyph& outGlyph) {
    const auto index = map.get(codepoint, 0xFFFFFFFFu);
    if (index != 0xFFFFFFFFu) {
        bmfont_glyph* glyph = file.glyphs + index;
        outGlyph.advanceWidth = glyph->advance_x;
        outGlyph.lineHeight = lineHeightMultiplier;
        sprite_t* spr = &RES_NAME_RESOLVE(res_sprite, glyph->sprite);
        if (spr->state & SPRITE_LOADED) {
            outGlyph.rect = spr->rect / file.header->base_font_size;
            outGlyph.texCoord = spr->tex;
            outGlyph.image = REF_RESOLVE(res_image, spr->image_id);
            outGlyph.rotated = spr->state & SPRITE_ROTATED;
        } else {
            outGlyph.rect = glyph->box;
        }
        outGlyph.source = this;
        return true;
    }
    return false;
}

bool BitmapFont::getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) {
    const auto index = map.get(codepoint, 0xFFFFFFFFu);
    if (index != 0xFFFFFFFFu) {
        bmfont_glyph* glyph = file.glyphs + index;
        outGlyph.advanceWidth = glyph->advance_x;
        outGlyph.lineHeight = lineHeightMultiplier;
        outGlyph.ascender = file.header->ascender;
        outGlyph.descender = file.header->descender;
        outGlyph.rect = glyph->box;
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