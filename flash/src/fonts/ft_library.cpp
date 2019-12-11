#include "ft_library.h"
#include <cassert>
#include <ek/util/logger.hpp>

namespace ek::font_lib {

ft2_context::ft2_context() {
    auto err = FT_Init_FreeType(&lib_);
    assert(err == 0);
    EK_TRACE("FreeType2 wrapper created");
}

ft2_context::~ft2_context() {
    FT_Done_FreeType(lib_);
    EK_TRACE("FreeType2 wrapper destroyed");
}

bool check(const std::vector<code_range_t>& ranges, uint32_t codepoint) {
    for (auto const& range : ranges) {
        if (codepoint >= range.min && codepoint <= range.max) return true;
    }
    return false;
}

ft2_face::ft2_face(const ft2_context& lib, const std::string& path) {
    int err = FT_New_Face(lib.handle(), path.c_str(), 0, &face_);
    assert(err == 0);
    EK_TRACE("FT2 Face NEW");
}

ft2_face::~ft2_face() {
    FT_Done_Face(face_);
    EK_TRACE("FT2 Face DELETE");
}

bool ft2_face::get_glyph_metrics(FT_UInt glyph_index, int* out_bbox, int* out_advance) const {
    FT_Error err = FT_Load_Glyph(face_, glyph_index, FT_LOAD_NO_SCALE);
    if (err == 0) {
        FT_GlyphSlot slot = face_->glyph;
        FT_Glyph glyph;
        FT_Get_Glyph(slot, &glyph);

        FT_BBox bbox;
        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_UNSCALED, &bbox);

        out_bbox[0] = int(bbox.xMin);
        out_bbox[1] = int(bbox.yMin);
        out_bbox[2] = int(bbox.xMax);
        out_bbox[3] = int(bbox.yMax);
        *out_advance = int(slot->advance.x);

        FT_Done_Glyph(glyph);

        return true;
    }
    return false;
}

std::vector<FT_ULong> ft2_face::get_available_char_codes(const std::vector<code_range_t>& ranges) const {
    std::vector<FT_ULong> ret;

    FT_UInt glyph_index;
    FT_ULong charcode = FT_Get_First_Char(face_, &glyph_index);
    while (glyph_index != 0) {
        if (check(ranges, charcode)) {
            ret.emplace_back(charcode);
        }
        charcode = FT_Get_Next_Char(face_, charcode, &glyph_index);
    }
    return ret;
}

rect_i ft2_face::get_glyph_bounds(FT_Face face) const {
    auto const& glyph = face->glyph;
    return rect_i{glyph->bitmap_left,
                  -glyph->bitmap_top,
                  (int) glyph->bitmap.width,
                  (int) glyph->bitmap.rows};
}

bool ft2_face::set_glyph_size(int font_size, float scale_factor) {
    auto dpi = static_cast<uint32_t>(72.0f * scale_factor);
    return FT_Set_Char_Size(face_, 0, font_size << 6, dpi, dpi) == 0;
}

}
