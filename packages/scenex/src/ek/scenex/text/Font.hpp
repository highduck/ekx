#pragma once

#include "FontImplBase.hpp"

#include <ek/math.h>
#include <ek/rr.h>

namespace ek {

struct Font {

    void draw(const char* text,
              float size,
              vec2_t position,
              color_t color,
              float line_height,
              float line_spacing = 0.0f) const;

    [[nodiscard]]
    float get_text_segment_width(const char* text, float size, int begin, int end) const;

    [[nodiscard]] FontType getFontType() const;

    void setFallbackFont(string_hash_t fallbackFont);

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph);

    bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph);

    void setBlur(float radius, int iterations, int strengthPower);

    FontImplBase* impl = nullptr;
    R(Font) fallback = 0;
};

}

struct res_font {
    string_hash_t names[16];
    ek::Font data[16];
    rr_man_t rr;
};

extern struct res_font res_font;

void setup_res_font(void);

#define R_FONT(name) REF_NAME(res_font, name)
