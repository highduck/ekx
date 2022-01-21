#pragma once

#include "Font.hpp"
#include "TextFormat.hpp"
#include <ek/ds/Array.hpp>

namespace ek {

struct TextBlockInfo {

    inline constexpr static int WarningLinesCount = 128;

    struct Line {
        int begin = 0;
        int end = 0;
        vec2_t size = {};
        float ascender = 0.0f;
        float descender = 0.0f;
        float leading = 0.0f;

        void updateSize(float length, float height);

        void close(float emptyLineHeight, int end);
    };

    // {max length; total height}
    vec2_t size = {};
    Array<Line> lines{};

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
    vec2_t position = {};

    // drawing zone
    rect_t bounds = {};

    // alignment for rect
    // TODO:
    vec2_t boundsAlignment = {};

    void draw(const char* text);

    void drawFormat(const char* fmt, ...);

    void drawWithBlockInfo(const char* text, const TextBlockInfo& info);

    void drawLayer(const char* text, const TextLayerEffect& layer, const TextBlockInfo& info) const;

    // bounds only for text lines (no glyph metrics or effect pixels)
    void getTextSize(const char* text, TextBlockInfo& info) const;
};

struct TextEngineSharedContext {
    TextEngine engine{};
    TextBlockInfo textBlockInfo{};
};

extern TextEngineSharedContext* gTextEngine;
void setup_text_engine(void);
TextEngineSharedContext* get_text_engine(void);

}

