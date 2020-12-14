#pragma once

#include <ek/util/assets.hpp>
#include "Font.hpp"
#include "TextFormat.hpp"

namespace ek {

struct TextBlockInfo {

    inline constexpr static int WarningLinesCount = 128;

    struct Line {
        int begin = 0;
        int end = 0;
        float2 size{};
        float ascender = 0.0f;
        float descender = 0.0f;
        float leading = 0.0f;

        void updateSize(float length, float height);

        void close(float emptyLineHeight, int end);
    };

    // {max length; total height}
    float2 size{};
    std::vector<Line> lines{};

    void addLine(Line line);

    void reset();

    void scale(float factor);

    [[nodiscard]] bool checkIsValid() const;
};

class TextEngine {
private:
    TextEngine() = default;

public:
    // user set
    TextFormat format{};
    float maxWidth = 0.0f;

    // current pen position
    float2 position{};

    // drawing zone
    rect_f bounds{};

    // alignment for rect
    // TODO:
    float2 boundsAlignment{};

    void draw(const char* text);

    void drawFormat(const char* fmt, ...);

    void drawWithBlockInfo(const char* text, const TextBlockInfo& info);

    void drawLayer(const char* text, const TextLayerEffect& layer, const TextBlockInfo& info) const;

    // bounds only for text lines (no glyph metrics or effect pixels)
    void getTextSize(const char* text, TextBlockInfo& info) const;

    static TextBlockInfo sharedTextBlockInfo;
    static TextEngine shared;
};


}

