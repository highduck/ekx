#pragma once

#include "FontImplBase.hpp"
#include <ek/math/Vec.hpp>
#include <ek/math/Color32.hpp>
#include <ek/math/Rect.hpp>
#include <ek/ds/Hash.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/LocalResource.hpp>

struct stbtt_fontinfo;

namespace ek {

class DynamicAtlas;
class MapFile;

// TODO: metadata for base size, atlas resolution, etc
// TODO: how to generate outlines?
class TrueTypeFont : public FontImplBase {
public:
    TrueTypeFont(float dpiScale, float fontSize, const char* dynamicAtlasName);

    ~TrueTypeFont() override;

    void loadDeviceFont(const char* fontName);

    void loadFromMemory(LocalResource lr);

    bool initFromMemory(const uint8_t* data, size_t size);

    void unload();

    bool getGlyph(uint32_t codepoint, Glyph& outGlyph) override;

    bool getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) override;

    float getKerning(uint32_t codepoint1, uint32_t codepoint2) override;

    void setBlur(float radius, int iterations, int strengthPower) override;

    void resetGlyphs();
public:
    stbtt_fontinfo* info = nullptr;
    LocalResource source{};

    float baseFontSize;
    float dpiScale;
    Res<DynamicAtlas> atlas;
    unsigned atlasVersion = 0;
    Hash<Glyph> map;
    uint64_t effectKeyBits = 0;

    float ascender = 0.0f;
    float descender = 0.0f;

    uint8_t blurRadius_ = 0;
    uint8_t blurIterations_ = 0;
    uint8_t strengthPower_ = 0;
};

}

