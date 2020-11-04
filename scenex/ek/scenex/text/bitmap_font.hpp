#pragma once

#include "font.hpp"
#include <vector>
#include <array>
#include <string>
#include <unordered_map>

namespace ek {


class font_glyph_t {
public:
    std::vector<uint32_t> codes;
    std::array<int, 4> box;
    int advance_width;
    std::string sprite;

    template<typename S>
    void serialize(IO<S>& io) {
        io(codes, box, advance_width, sprite);
    }
};

class font_data_t {
public:
    uint16_t units_per_em;
    std::vector<font_glyph_t> glyphs;
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

    void load(const font_data_t& data);

    bool getGlyph(uint32_t codepoint, const FontSizeInfo& size, Glyph& outGlyph) override;

    FontSizeInfo getSizeInfo(float size) override;

public:
    std::vector<uint16_t> bitmap_size_table;
    std::unordered_map<uint32_t, font_glyph_t> map;
    uint16_t units_per_em{};
};


}

