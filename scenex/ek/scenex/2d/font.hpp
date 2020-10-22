#pragma once

#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/box.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/serialize/serialize.hpp>

#include <unordered_map>
#include <vector>
#include <array>
#include <string>

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

//struct bitmap_font_size_t {
//    float size = 0.0f;
//    std::unordered_map<uint32_t, asset_t<sprite_t>> map;
//};

class font_t : private disable_copy_assign_t {
public:
    std::vector<uint16_t> bitmap_size_table;
    std::unordered_map<uint32_t, font_glyph_t> map;
    uint16_t units_per_em;

    explicit font_t(const font_data_t& data);

    [[nodiscard]]
    const font_glyph_t* get_glyph(uint32_t code) const;

    void draw(const std::string& text,
              float size,
              const float2& position,
              argb32_t color,
              float line_height,
              float line_spacing = 0.0f) const;

    [[nodiscard]]
    float get_text_segment_width(const std::string& text, float size, int begin, int end) const;

//    rect_f getBoundingBox(const std::string& text, float size, int begin, int end) const;

    [[nodiscard]]
    rect_f get_line_bounding_box(const std::string& text, float size, int begin, int end, float line_height,
                                 float line_spacing = 0.0f) const;

    [[nodiscard]]
    rect_f estimate_text_draw_zone(const std::string& text, float size, int begin, int end, float line_height,
                                   float line_spacing = 0.0f) const;
};

font_t* load_font(const std::vector<uint8_t>& buffer);

}
