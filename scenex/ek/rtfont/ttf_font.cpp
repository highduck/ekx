#include "ttf_font.hpp"

#include <unordered_map>
#include <ek/app/res.hpp>
#include <ek/draw2d/drawer.hpp>

#ifndef STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#endif

#include "stb_truetype.h"
#include "dynamic_atlas.hpp"

namespace ek {

TTFFont::~TTFFont() {
    unload();
}


void TTFFont::loadFromMemory(array_buffer&& buffer) {
    assert(!loaded);
    const auto* fontData = buffer.data();
    const auto fontIndex = 0;
    const int fontOffset = stbtt_GetFontOffsetForIndex(fontData, fontIndex);
    assert(fontOffset >= 0 && "fontData is incorrect, or fontIndex cannot be found.");
    if (stbtt_InitFont(&info, fontData, fontOffset)) {
        source = std::move(buffer);
        loaded = true;
    }
}

void TTFFont::unload() {
    if (loaded) {
        source.resize(0);
        source.shrink_to_fit();
        loaded = false;
        info = {};
    }
}

Glyph scaleGlyph(const Glyph& glyph, float scale) {
    auto r = glyph;
    r.advanceWidth *= scale;
    r.leftSideBearing *= scale;
    r.x0 *= scale;
    r.x1 *= scale;
    r.y0 *= scale;
    r.y1 *= scale;
    return r;
}

bool TTFFont::getGlyph(uint32_t codepoint, const FontSizeInfo& sizeInfo, Glyph& outGlyph) {
    auto it = map_.find(codepoint);
    if (it != map_.end()) {
        outGlyph = scaleGlyph(it->second, sizeInfo.rasterScale);
        return true;
    }
    auto& glyph = map_.try_emplace(codepoint).first->second;

    const auto* font = &info;
    const auto scale = stbtt_ScaleForPixelHeight(font, fontSize);
    const auto glyphIndex = stbtt_FindGlyphIndex(font, codepoint);
    if (!glyphIndex) {
        /* @rlyeh: glyph not found, ie, arab chars */
        return false;
    }

    int advanceWidth = 0, leftSideBearing = 0;
    int x0, y0, x1, y1;

    stbtt_GetGlyphHMetrics(font, glyphIndex, &advanceWidth, &leftSideBearing);
    stbtt_GetGlyphBitmapBox(font, glyphIndex, scale_ * scale, scale_ * scale, &x0, &y0, &x1, &y1);

    glyph.advanceWidth = scale * advanceWidth;
    glyph.leftSideBearing = scale * leftSideBearing;
    glyph.size = fontSize;

    auto glyphWidth = x1 - x0;
    auto glyphHeight = y1 - y0;

    std::vector<uint8_t> bmp;
    bmp.resize(glyphWidth * glyphHeight);

    stbtt_MakeGlyphBitmap(font, bmp.data(), glyphWidth, glyphHeight, glyphWidth, scale_ * scale, scale_ * scale,
                          glyphIndex);
    std::vector<uint8_t> bmp2;
    bmp2.resize(4 * glyphWidth * glyphHeight);

    for (size_t i = 0, j = 0; i < bmp.size(); ++i) {
        const auto c = bmp[i];
        bmp2[j++] = c;
        bmp2[j++] = c;
        bmp2[j++] = c;
        bmp2[j++] = c;
    }

    auto sprite = atlas.addBitmap(glyphWidth, glyphHeight, bmp2);
    glyph.texture = sprite.texture;
    glyph.texCoord = sprite.texCoords;
    glyph.x0 = x0 / scale_;
    glyph.x1 = x1 / scale_;
    glyph.y0 = y0 / scale_;
    glyph.y1 = y1 / scale_;

    outGlyph = scaleGlyph(glyph, sizeInfo.rasterScale);
    return true;
}

FontSizeInfo TTFFont::getSizeInfo(float size) {
    FontSizeInfo info;
    info.rasterScale = size / fontSize;
    info.metricsScale = 1.0f;
    info.baseFontSize = fontSize;
    return info;
}

void TTFFont::debugDrawAtlas() {
    draw2d::state.set_empty_texture();
    draw2d::quad(100, 100, 1024, 1024, 0x77000000_argb);

    draw2d::state.set_texture(atlas.texture_);
    draw2d::state.set_texture_coords(0, 0, 1, 1);
    draw2d::quad(100, 100, 1024, 1024);
}

void TTFFont::setScale(float scale) {
    scale_ = scale;
}

FontType TTFFont::getType() const {
    return FontType::TrueType;
}

}