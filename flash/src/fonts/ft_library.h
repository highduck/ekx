#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <vector>
#include <string>

#include <ek/math/box.hpp>

namespace ek::font_lib {

struct code_range_t {
    uint32_t min;
    uint32_t max;
};

class ft2_context {
public:
    ft2_context();

    ~ft2_context();

    [[nodiscard]]
    FT_Library handle() const {
        return lib_;
    }

private:
    FT_Library lib_{};
};

class ft2_face {
public:
    ft2_face(const ft2_context& lib, const std::string& path);

    ~ft2_face();

    [[nodiscard]]
    FT_Face data() const { return face_; }

    bool get_glyph_metrics(FT_UInt glyph_index, int* out_bbox, int* out_advance) const;

    [[nodiscard]]
    std::vector<FT_ULong> get_available_char_codes(const std::vector<code_range_t>& ranges) const;

    rect_i get_glyph_bounds(FT_Face face) const;

    bool set_glyph_size(int font_size, float scale_factor);

private:
    FT_Face face_{};
};
}