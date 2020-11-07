#pragma once

#include "font_base.hpp"
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <ek/math/serialize_math.hpp>
#include <ek/util/common_macro.hpp>

namespace ek {

class BitmapFontGlyph {
public:
    std::vector<uint32_t> codes;
    rect_i box;
    int advance_width;
    std::string sprite;

    template<typename S>
    void serialize(IO<S>& io) {
        io(codes, box, advance_width, sprite);
    }
};

class BitmapFontData {
public:
    uint16_t units_per_em;
    std::vector<BitmapFontGlyph> glyphs;
    std::vector<uint16_t> sizes;

    template<typename S>
    void serialize(IO<S>& io) {
        io(units_per_em, sizes, glyphs);
    }
};

class BitmapFont : public FontImplBase, private disable_copy_assign_t {
public:

    BitmapFont();

    ~BitmapFont() override;

    void load(const std::vector<uint8_t>& buffer);

    void load(const BitmapFontData& data);

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph) override;

    void setFontSize(float fontSize) override;

public:
    std::vector<uint16_t> bitmap_size_table;
    std::unordered_map<uint32_t, BitmapFontGlyph> map;
    uint16_t units_per_em{};

    float metricsScale;
    float rasterScale;
    uint16_t baseFontSize;
};

}

