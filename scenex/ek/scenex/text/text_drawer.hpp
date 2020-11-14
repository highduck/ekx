#pragma once

#include <ek/util/assets.hpp>
#include "font.hpp"
#include "text_format.hpp"

namespace ek {

struct TextBlockInfo {
    // {max length; total height}
    float2 size{};
    float2 line[128]{};
    int numLines = 0;

    void pushLine(float emptyLineHeight);

    void reset();

    void updateLine(float length, float height);
};

class TextDrawer {
public:
    // user set
    TextFormat format{};

    // current pen position
    float2 position{};

    // drawing zone
    rect_f bounds{};

    // alignment for rect
    // TODO:
    float2 boundsAlignment{};

    void draw(const char* text);

    void drawWithBlockInfo(const char* text, const TextBlockInfo& info);

    void drawLayer(const char* text, const TextLayerEffect& layer, const TextBlockInfo& info) const;

    // bounds only for text lines (no glyph metrics or effect pixels)
    void getTextSize(const char* text, TextBlockInfo& info) const;

    static TextBlockInfo sharedTextBlockInfo;
};


}

