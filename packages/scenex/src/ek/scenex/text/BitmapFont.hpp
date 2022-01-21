#pragma once

#include "FontImplBase.hpp"
#include <ek/scenex/data/BMFont.hpp>
#include <ek/ds/Array.hpp>
#include <ek/ds/Hash.hpp>

namespace ek {

class BitmapFont : public FontImplBase {
public:

    BitmapFont();

    ~BitmapFont() override;

    void load(const uint8_t* buffer, size_t length);

    void load(BMFont& data);

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph) override;

    bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) override;

    float getKerning(uint32_t codepoint1, uint32_t codepoint2) override;

public:
    Array<BMFontGlyph> glyphs;
    Hash<BMFontGlyph*> map;
    float unitsPerEM = 1.0f;
    float baseFontSize = 1.0f;
    float baseFontSizeInv = 1.0f;
    float lineHeight = 0.0f;
    float ascender = 0.0f;
    float descender = 0.0f;
};

}

