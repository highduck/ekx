#include "ft_library.hpp"
#include <cassert>
#include <ek/util/logger.hpp>

namespace ek::font_lib {

FreeType2::FreeType2() {
    auto err = FT_Init_FreeType(&lib_);
    if (err != 0) {
        EK_ERROR("FreeType2 init error: %d", err);
    }
    EK_TRACE("FreeType2 wrapper created");
}

FreeType2::~FreeType2() {
    FT_Done_FreeType(lib_);
    EK_TRACE("FreeType2 wrapper destroyed");
}

bool check(const std::vector<std::pair<uint32_t, uint32_t>>& ranges, uint32_t codepoint) {
    for (const auto& range : ranges) {
        if (codepoint >= range.first && codepoint <= range.second) {
            return true;
        }
    }
    return false;
}

FontFace::FontFace(const FreeType2& lib, const std::string& path) {
    int err = FT_New_Face(lib.handle(), path.c_str(), 0, &face_);
    if (err != 0) {
        EK_ERROR("FT2 new face error: %d", err);
    }

    EK_TRACE("FT2 Face NEW");
}

FontFace::~FontFace() {
    FT_Done_Face(face_);
    EK_TRACE("FT2 Face DELETE");
}

bool FontFace::getGlyphMetrics(uint32_t glyphIndex, int* out_bbox, int* out_advance) const {
    FT_Error err = FT_Load_Glyph(face_, glyphIndex, FT_LOAD_NO_SCALE);
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

std::vector<uint32_t> FontFace::getAvailableCodepoints(const std::vector<std::pair<uint32_t, uint32_t>>& ranges) const {
    std::vector<uint32_t> ret;

    FT_UInt glyphIndex;
    FT_ULong codepoint = FT_Get_First_Char(face_, &glyphIndex);
    while (glyphIndex != 0) {
        if (check(ranges, codepoint)) {
            ret.emplace_back(codepoint);
        }
        codepoint = FT_Get_Next_Char(face_, codepoint, &glyphIndex);
    }
    return ret;
}

bool FontFace::setGlyphSize(int fontSize, float scaleFactor) {
    // TODO:
    //  - make scaling to match freetype/stb openFrameworks/graphics/ofTrueTypeFont.cpp
    auto dpi = static_cast<uint32_t>(72.0f * scaleFactor);
    return FT_Set_Char_Size(face_, 0, fontSize << 6, dpi, dpi) == 0;
}

bool FontFace::loadGlyph(uint32_t glyphIndex) const {
    auto err = FT_Load_Glyph(face_, glyphIndex, FT_LOAD_DEFAULT);
    return err == 0;
}

bool FontFace::renderGlyph(uint8_t** buffer, uint32_t* width, uint32_t* height) const {
    auto err = FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_NORMAL);
    if (err == 0) {
        auto bitmap = face_->glyph->bitmap;
        if (bitmap.buffer) {
            assert(bitmap.width != 0 && bitmap.rows != 0);
            if (buffer) {
                *buffer = bitmap.buffer;
            }
            if (width) {
                *width = bitmap.width;
            }
            if (height) {
                *height = bitmap.rows;
            }
            return true;
        }
    }
    return false;
}

rect_i FontFace::getGlyphBounds() const {
    auto const& glyph = face_->glyph;
    return rect_i{glyph->bitmap_left,
                  -glyph->bitmap_top,
                  (int) glyph->bitmap.width,
                  (int) glyph->bitmap.rows};
}

uint32_t FontFace::getGlyphIndex(uint32_t codepoint) const {
    return FT_Get_Char_Index(face_, codepoint);
}

bool FontFace::getKerning(uint32_t index1, uint32_t index2, int* x, int* y) const {
    FT_Vector kern{};
    auto res = FT_Get_Kerning(face_, index1, index2, FT_KERNING_UNSCALED, &kern);
    if (res == 0) {
        if (x) {
            *x = kern.x;
        }
        if (y) {
            *y = kern.y;
        }
        return true;
    }
    return false;
}

FontFaceInfo FontFace::getInfo() const {
    FontFaceInfo info{};
    info.unitsPerEM = face_->units_per_EM;
    info.ascender = face_->ascender;
    info.descender = face_->descender;
    info.lineHeight = face_->height;
    info.hasKerning = FT_HAS_KERNING(face_) != 0;
    return info;
}

}
