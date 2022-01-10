#pragma once

#include "FontImplBase.hpp"

#include <ek/math.h>
#include <ek/math.h>
#include <ek/util/NoCopyAssign.hpp>
#include <ek/serialize/serialize.hpp>
#include <ek/util/Type.hpp>

#include <ek/ds/String.hpp>

namespace ek {

class Font : private NoCopyAssign {
public:
    explicit Font(FontImplBase* impl);

    ~Font();

    void draw(const char* text,
              float size,
              vec2_t position,
              color_t color,
              float line_height,
              float line_spacing = 0.0f) const;

    [[nodiscard]]
    float get_text_segment_width(const char* text, float size, int begin, int end) const;

    [[nodiscard]] FontType getFontType() const;

    [[nodiscard]] FontImplBase* getImpl();

    [[nodiscard]] const FontImplBase* getImpl() const;

    void setFallbackFont(Font* fallbackFont) {
        fallback = fallbackFont;
    }

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph);

    bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph);

    void setBlur(float radius, int iterations, int strengthPower);
private:
    FontImplBase* impl = nullptr;
    Font* fallback = nullptr;
};

EK_DECLARE_TYPE(Font);
EK_TYPE_INDEX(Font, 3);

}
