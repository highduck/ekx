#pragma once

#include <ek/ds/Array.hpp>
#include <ek/math/box.hpp>
#include <string>
#include <ek/serialize/serialize.hpp>

namespace ek {

class BMFontGlyph {
public:
    Array <uint32_t> codepoints;
    rect_t <int32_t> box;
    int32_t advanceWidth;
    std::string sprite;

    template<typename S>
    void serialize(IO <S>& io) {
        io(codepoints, box, advanceWidth, sprite);
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

    template<typename S>
    void serialize(IO <S>& io) {
        io(unitsPerEM, fontSize, lineHeight, ascender, descender, glyphs);
    }
};

}