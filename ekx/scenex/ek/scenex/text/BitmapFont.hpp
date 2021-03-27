#pragma once

#include "FontImplBase.hpp"
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <ek/math/serialize_math.hpp>
#include <ek/util/common_macro.hpp>

namespace ek {

class BitmapFontGlyph {
public:
    std::vector<uint32_t> codepoints;
    rect_t<int32_t> box;
    int32_t advanceWidth;
    std::string sprite;

    template<typename S>
    void serialize(IO<S>& io) {
        io(codepoints, box, advanceWidth, sprite);
    }
};

class BitmapFontData {
public:
    // 16 bits, but we need align memory for reading buffer
    int32_t unitsPerEM;
    int32_t fontSize;
    int32_t lineHeight;
    int32_t ascender;
    int32_t descender;
    std::vector<BitmapFontGlyph> glyphs;

    template<typename S>
    void serialize(IO<S>& io) {
        io(unitsPerEM, fontSize, lineHeight, ascender, descender, glyphs);
    }
};

class BitmapFont : public FontImplBase, private disable_copy_assign_t {
public:

    BitmapFont();

    ~BitmapFont() override;

    void load(const std::vector<uint8_t>& buffer);

    void load(const BitmapFontData& data);

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph) override;

    bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) override;

    float getKerning(uint32_t codepoint1, uint32_t codepoint2) override;

public:
    std::unordered_map<uint32_t, BitmapFontGlyph> map;
    float unitsPerEM = 1.0f;
    float baseFontSize = 1.0f;
    float baseFontSizeInv = 1.0f;
    float lineHeight = 0.0f;
    float ascender = 0.0f;
    float descender = 0.0f;
};

}
