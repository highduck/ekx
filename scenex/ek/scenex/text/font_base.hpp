#pragma once

#include <ek/math/box.hpp>

namespace ek {

namespace graphics {
class texture_t;
}

struct Glyph {
    rect_f rect{};
    rect_f texCoord{};
    float advanceWidth{};
    const graphics::texture_t* texture = nullptr;
    bool rotated = false;

    Glyph& scale(float factor) {
        rect *= factor;
        advanceWidth *= factor;
        return *this;
    }
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

    virtual void setBlur(float radius, int iterations, int strengthPower) {
        // by default blur is not supported
    }

    virtual void setFontSize(float fontSize) = 0;

    virtual void debugDrawAtlas(float x, float y) {}

    [[nodiscard]] FontType getFontType() const {
        return fontType;
    }

    [[nodiscard]] float getLineHeight(float fontSize) const {
        return fontSize * lineHeightMultiplier;
    }

protected:
    FontType fontType;
    float lineHeightMultiplier;
};

}
