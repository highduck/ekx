#pragma once

#include "font_base.hpp"
#include "dynamic_atlas.hpp"
#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/box.hpp>
#include <unordered_map>
#include <ek/draw2d/drawer.hpp>

struct stbtt_fontinfo;

namespace ek {

class FileView;

// TODO: metadata for base size, atlas resolution, etc
// TODO: how to generate outlines?
class TrueTypeFont : public FontImplBase {
public:
    TrueTypeFont(float dpiScale, float fontSize, int cachePageSize);

    ~TrueTypeFont() override;

    void loadDeviceFont(const char* fontName);

    void loadFromMemory(std::vector<uint8_t>&& buffer);

    bool initFromMemory(const uint8_t* data, size_t size);

    void unload();

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph) override;

    bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) override;

    float getKerning(uint32_t codepoint1, uint32_t codepoint2) override;

    void setBlur(float radius, int iterations, int strengthPower) override;

    void debugDrawAtlas(float x, float y) override;

public:
    stbtt_fontinfo* info = nullptr;
    std::vector<uint8_t> source;
    FileView* mappedSourceFile_ = nullptr;
    bool loaded = false;

    float baseFontSize;
    float dpiScale;
    DynamicAtlas atlas;
    std::unordered_map<uint32_t, std::unique_ptr<std::unordered_map<uint32_t, Glyph>>> mapByEffect;
    std::unordered_map<uint32_t, Glyph>* map = nullptr;

    float ascender = 0.0f;
    float descender = 0.0f;

    uint8_t blurRadius_ = 0;
    uint8_t blurIterations_ = 0;
    uint8_t strengthPower_ = 0;
};

}

