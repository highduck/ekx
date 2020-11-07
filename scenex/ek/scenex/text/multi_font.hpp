#pragma once

#include "font_base.hpp"
#include "font.hpp"
#include <ek/util/assets.hpp>

namespace ek {

class MultiFont : public FontImplBase {
public:
    MultiFont();

    ~MultiFont() override;

    void addFont(const char* name);

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph) override;

    void setBlur(float radius, int iterations, int strengthPower) override;

public:

    // source fonts to use in order to fetch
    std::vector<asset_t<Font>> fonts;

    float blurRadius_ = 0.0f;
    uint8_t blurIterations_ = 0;
    uint8_t strengthPower_ = 0;
};

}

