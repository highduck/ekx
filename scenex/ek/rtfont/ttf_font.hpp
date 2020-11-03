#pragma once

#include <ek/app/res.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/box.hpp>
#include <unordered_map>
#include <ek/scenex/2d/font.hpp>
#include <ek/draw2d/drawer.hpp>
#include "dynamic_atlas.hpp"
#include "stb_truetype.h"

namespace ek {

// TODO: metadata for base size, atlas resolution, etc
// TODO: how to generate outlines?
class TTFFont : public FontImplBase {
    stbtt_fontinfo info{};
    std::vector<uint8_t> source;
    bool loaded = false;

    int fontSize = 48;
    DynamicAtlas atlas{512, 512};
    std::unordered_map<uint32_t, Glyph> map_;
    float scale_ = 1.0f;

public:
    TTFFont() = default;
    ~TTFFont();

    void loadFromMemory(array_buffer&& buffer);

    void unload();

    bool getGlyph(uint32_t codepoint, const FontSizeInfo& sizeInfo, Glyph& outGlyph) override;

    FontSizeInfo getSizeInfo(float size) override;

    void debugDrawAtlas() override;

    void setScale(float D);

    FontType getType() const override;
};

}

