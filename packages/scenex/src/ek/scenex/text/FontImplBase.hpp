#pragma once

#include <ek/math/Rect.hpp>

namespace ek {

namespace graphics {
class Texture;
}

class FontImplBase;

struct Glyph {
    // multiply by fontSize
    Rect2f rect{};
    float advanceWidth{};
    float bearingX{};
    float lineHeight{};
    float ascender = 0.0f;
    float descender = 0.0f;

    // sprite part
    Rect2f texCoord{};
    const graphics::Texture* texture = nullptr;
    bool rotated = false;

    // we need to know what fallback give us this glyph info,
    // then we can ask additional info directly from source font implementation,
    // for example: kerning information
    FontImplBase* source = nullptr;
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
        (void)radius;
        (void)iterations;
        (void)strengthPower;
        // by default blur is not supported
    }

    [[nodiscard]] FontType getFontType() const {
        return fontType;
    }

    bool isReady() const;

protected:
    FontType fontType;
    float lineHeightMultiplier;

    bool loaded_ = false;
    bool ready_ = false;
};

}
