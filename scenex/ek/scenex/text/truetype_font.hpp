#pragma once

#include <ek/app/res.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/box.hpp>
#include <unordered_map>
#include "font.hpp"
#include <ek/draw2d/drawer.hpp>
#include "dynamic_atlas.hpp"
#include "stb_truetype.h"

namespace ek {

// TODO: metadata for base size, atlas resolution, etc
// TODO: how to generate outlines?
class TrueTypeFont : public FontImplBase {
public:
    TrueTypeFont(float dpiScale, float fontSize, int cachePageSize);

    ~TrueTypeFont() override;

    void loadFromMemory(array_buffer&& buffer);

    void unload();

    bool getGlyph(uint32_t codepoint, const FontSizeInfo& sizeInfo, Glyph& outGlyph) override;

    FontSizeInfo getSizeInfo(float size) override;

    void debugDrawAtlas() override;
public:
    stbtt_fontinfo info{};
    std::vector<uint8_t> source;
    bool loaded = false;

    float baseFontSize;
    float dpiScale;
    DynamicAtlas atlas;
    std::unordered_map<uint32_t, Glyph> map_;

    float ascender = 0.0f;
    float descender = 0.0f;
};

}

