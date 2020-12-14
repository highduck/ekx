#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <vector>
#include <string>

#include <ek/math/box.hpp>

namespace ek::font_lib {

class FreeType2 {
public:
    FreeType2();

    ~FreeType2();

    [[nodiscard]]
    FT_Library handle() const {
        return lib_;
    }

private:
    FT_Library lib_{};
};

struct FontFaceInfo {
    uint16_t unitsPerEM;
    int16_t ascender;
    int16_t descender;
    int16_t lineHeight;
    bool hasKerning;
};

class FontFace {
public:
    FontFace(const FreeType2& lib, const std::string& path);

    ~FontFace();

    [[nodiscard]]
    FT_Face data() const { return face_; }

    bool getGlyphMetrics(uint32_t glyphIndex, int* out_bbox, int* out_advance) const;

    [[nodiscard]]
    std::vector<uint32_t> getAvailableCodepoints(const std::vector<std::pair<uint32_t, uint32_t>>& ranges) const;

    bool setGlyphSize(int fontSize, float scaleFactor);

    uint32_t getGlyphIndex(uint32_t codepoint) const;

    bool loadGlyph(uint32_t glyphIndex) const;

    bool renderGlyph(uint8_t** buffer, uint32_t* width, uint32_t* height) const;

    rect_i getGlyphBounds() const;

    bool getKerning(uint32_t index1, uint32_t index2, int* x, int* y) const;

    FontFaceInfo getInfo() const;
private:
    FT_Face face_{};
};

}