#pragma once

#include <ek/ds/Array.hpp>
#include <ek/ds/Hash.hpp>
#include <ek/math/Rect.hpp>

#include <ek/serialize/serialize.hpp>
#include <ek/serialize/stl/String.hpp>

namespace ek {

class BMFontGlyph {
public:
    Rect2<int32_t> box;
    int32_t advanceWidth;
    String sprite;

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
    Hash <uint32_t> codepoints;

    template<typename S>
    void serialize(IO <S>& io) {
        io(unitsPerEM, fontSize, lineHeight, ascender, descender, glyphs, codepoints);
    }
};

}