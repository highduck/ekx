#pragma once

#include <ek/math/box.hpp>

namespace ek {

namespace graphics {
class texture_t;
}

struct Glyph {
    // multiply by fontSize
    rect_f rect{};
    float advanceWidth{};
    float bearingX{};
    float lineHeight{};

    // sprite part
    rect_f texCoord{};
    const graphics::texture_t* texture = nullptr;
    bool rotated = false;
};

enum class FontType {
    Bitmap,
    TrueType
};

class FontImplBase {
public:
    explicit FontImplBase(FontType type);

    virtual ~FontImplBase() = 0;

    virtual bool getGlyph(uint32_t codepoint, Glyph& outGlyph) = 0;

    virtual bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) = 0;

    virtual float getKerning(uint32_t codepoint1, uint32_t codepoint2) = 0;

    virtual void setBlur(float radius, int iterations, int strengthPower) {
        // by default blur is not supported
    }

    virtual void debugDrawAtlas(float x, float y) {}

    [[nodiscard]] FontType getFontType() const {
        return fontType;
    }

protected:
    FontType fontType;
    float lineHeightMultiplier;
};

}
