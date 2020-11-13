#pragma once

#include "font_base.hpp"

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

class Font : private disable_copy_assign_t {
public:
    explicit Font(FontImplBase* impl);

    ~Font();

    void debugDrawAtlas(float x, float y);

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

    [[nodiscard]] FontType getFontType() const;

    [[nodiscard]] FontImplBase* getImpl();

    [[nodiscard]] const FontImplBase* getImpl() const;

    void setFallbackFont(Font* fallbackFont) {
        fallback = fallbackFont;
    }

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph);

    bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph);

    float getKerning(uint32_t codepoint1, uint32_t codepoint2);

    void setBlur(float radius, int iterations, int strengthPower);
private:
    FontImplBase* impl = nullptr;
    Font* fallback = nullptr;
};

}