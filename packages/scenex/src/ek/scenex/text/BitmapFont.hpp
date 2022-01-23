#pragma once

#include "FontImplBase.hpp"
#include <ek/format/bmfont.h>
#include <ek/ds/Array.hpp>
#include <ek/ds/Hash.hpp>

namespace ek {

class BitmapFont : public FontImplBase {
public:

    BitmapFont();

    ~BitmapFont() override;

    void load(const uint8_t* buffer, size_t length);

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph) override;

    bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) override;

    float getKerning(uint32_t codepoint1, uint32_t codepoint2) override;

public:
    // bmfont is mapped to memory, we use this control structure with mapped pointers as source of data
    bmfont_file file = {};
    // dynamic hash-map to map 32-bit codepoint to 32-bit glyph index
    Hash<uint32_t> map;
};

}

