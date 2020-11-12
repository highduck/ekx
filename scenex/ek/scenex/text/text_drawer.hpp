#pragma once

#include <ek/util/assets.hpp>
#include "font.hpp"
#include "text_format.hpp"

namespace ek {

struct TextRenderPass {
    argb32_t color = 0xFFFFFFFF_argb;
    float2 offset{};
    float blurRadius = 0.0f;
    uint8_t blurIterations = 0;
    uint8_t filterStrength = 0;
    bool showGlyphBounds = false;
};

struct TextBlockInfo {
    rect_f bounds{};
    float lineLength[128]{};
    float lineHeight[128]{};
    float maxLength{};
    float totalHeight{};
    int numLines = 0;

    void pushLine(float emptyLineHeight);

    void reset();

    void updateLineLength(float length);
    void updateLineHeight(float height);
};

class TextDrawer {
public:
    // user set
    TextFormat format{};

    // drawing zone
    rect_f rect{};

    // alignment for rect
    float2 alignment{};

    // internal renderer state
    asset_t<Font> font;
    float size = 16.0f;
    float leading = 0.0f;
    float letterSpacing = 0.0f;
    bool kerning = true;

    TextRenderPass pass{};

    void draw(const char* text);

    void drawPass(const char* text, const TextBlockInfo& info);

    // bounds only for text lines (no glyph metrics or effect pixels)
    void getTextSize(const char* text, TextBlockInfo& info);

    static TextBlockInfo sharedTextBlockInfo;
};


}

