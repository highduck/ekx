#pragma once

#include "FontImplBase.hpp"
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <ek/math/serialize_math.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/scenex/data/BMFont.hpp>
#include <ek/util/Type.hpp>

namespace ek {

class BitmapFont : public FontImplBase, private disable_copy_assign_t {
public:

    BitmapFont();

    ~BitmapFont() override;

    void load(const std::vector<uint8_t>& buffer);

    void load(const BMFont& data);

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph) override;

    bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) override;

    float getKerning(uint32_t codepoint1, uint32_t codepoint2) override;

public:
    std::unordered_map<uint32_t, BMFontGlyph> map;
    float unitsPerEM = 1.0f;
    float baseFontSize = 1.0f;
    float baseFontSizeInv = 1.0f;
    float lineHeight = 0.0f;
    float ascender = 0.0f;
    float descender = 0.0f;
};

}

