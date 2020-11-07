#pragma once

#include <ek/math/box.hpp>

namespace ek {

namespace graphics {
class texture_t;
}

struct Glyph {
    rect_f texCoord{};

    // multiply by fontSize
    rect_f rect{};
    float advanceWidth{};
    float lineHeight{};

    const graphics::texture_t* texture = nullptr;
    bool rotated = false;
};

enum class FontType {
    Bitmap,
    TrueType,
    Multi
};

class FontImplBase {
public:
    explicit FontImplBase(FontType type);

    virtual ~FontImplBase() = 0;

    virtual bool getGlyph(uint32_t codepoint, Glyph& outGlyph) = 0;

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
