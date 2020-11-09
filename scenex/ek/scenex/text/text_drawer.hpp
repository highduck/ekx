#pragma once

#include <ek/util/assets.hpp>
#include "font.hpp"
#include "text_format.hpp"

namespace ek {

struct TextLayerEffect {
    argb32_t color = 0xFFFFFFFF_argb;
    float2 offset{};
    float blurRadius = 0.0f;
    uint8_t blurIterations = 0;
    uint8_t strength = 0;
    bool glyphBounds = false;
};

struct TextFormat {
    asset_t<Font> font;
    float size = 16.0f;
    float leading = 0.0f;
    float letterSpacing = 0.0f;
    bool kerning = true;
    bool underline = false;
    bool strikethrough = false;

    TextLayerEffect layers[4]{};
    int layersCount = 1;
};

struct TextRenderPass {
    argb32_t color = 0xFFFFFFFF_argb;
    float2 offset{};
    float blurRadius = 0.0f;
    uint8_t blurIterations = 0;
    uint8_t filterStrength = 0;
};

class TextDrawer {
public:
    // user set
    TextFormat format{};

    float2 position{};
    float2 rectAlignment{};

    // internal renderer state
    asset_t<Font> font;
    float size = 16.0f;
    float leading = 0.0f;
    float letterSpacing = 0.0f;
    bool kerning = true;

    TextRenderPass pass{};

    void draw(const std::string& text);
    void drawPass(const std::string& text);
    void drawGlyphBounds(const std::string& text);
};


}

