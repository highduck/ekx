#pragma once

#include <ek/util/assets.hpp>
#include "font.hpp"
#include "text_format.hpp"

namespace ek {

struct FastBlurEffect {

};

struct TextEffect {

};

struct TextDrawingState {
    asset_t<Font> font;
    asset_t<Font> nativeFont;
};

class TextDrawer {
public:
    float2 position{};
    float fontSize = 16.0f;
    argb32_t textColor = 0xFFFFFFFF_argb;
    float2 alignment;
    float2 offset;
    float lineSpacing = 0.0f;
    float letterSpacing = 0.0f;
    bool kerning = true;
    asset_t<Font> font;
    asset_t<Font> nativeFont;

    void draw(const std::string& text) const;

    void setBlur(float radius, int iterations, int strengthPower);
};


}

