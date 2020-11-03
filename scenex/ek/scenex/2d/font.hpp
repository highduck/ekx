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

namespace graphics {
class texture_t;
}

struct Glyph {
    float x0, y0, x1, y1;
    float advanceWidth;
    float leftSideBearing;
    int size;
    rect_f texCoord;
    const graphics::texture_t* texture;
    bool rotated;
};

struct FontSizeInfo {
    float metricsScale;
    float rasterScale;
    uint16_t baseFontSize;
};

enum class FontType {
    Bitmap,
    TrueType
};

class FontImplBase {
public:
    virtual ~FontImplBase() = 0;

    virtual bool getGlyph(uint32_t codepoint, const FontSizeInfo& size, Glyph& outGlyph) = 0;

    virtual FontSizeInfo getSizeInfo(float size) = 0;

    virtual void debugDrawAtlas() {}

    virtual FontType getType() const = 0;
};

class font_t : private disable_copy_assign_t {
public:
    explicit font_t(FontImplBase* impl);

    ~font_t();

    void debugDrawAtlas();

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

    [[nodiscard]] FontType getType() const;

    [[nodiscard]] const FontImplBase* getImpl() const;

private:
    FontImplBase* impl = nullptr;
};

}
