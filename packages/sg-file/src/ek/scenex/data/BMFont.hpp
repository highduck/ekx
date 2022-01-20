#pragma once

#include <ek/ds/Array.hpp>
#include <ek/math.h>

#include <ek/serialize/serialize.hpp>

namespace ek {

class BMFontGlyph {
public:
    irect_t box;
    int32_t advanceWidth;
    string_hash_t sprite;

    template<typename S>
    void serialize(IO <S>& io) {
        io(box, advanceWidth, sprite);
    }
};

class BMFont {
public:
    // 16 bits, but we need align memory for reading buffer
    int32_t unitsPerEM;
    int32_t fontSize;
    int32_t lineHeight;
    int32_t ascender;
    int32_t descender;
    Array <BMFontGlyph> glyphs;

    // (codepoint: u32 | glyph_index: u32)
    Array <uint32_t> codepoints;

    template<typename S>
    void serialize(IO <S>& io) {
        io(unitsPerEM, fontSize, lineHeight, ascender, descender, glyphs, codepoints);
    }
};

}