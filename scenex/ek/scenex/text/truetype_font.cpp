#include "truetype_font.hpp"

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

TrueTypeFont::TrueTypeFont(float dpiScale_, float fontSize, int cachePageSize) :
        FontImplBase(FontType::TrueType),
        dpiScale{dpiScale_},
        baseFontSize{fontSize},
        atlas{cachePageSize, cachePageSize} {

}

TrueTypeFont::~TrueTypeFont() {
    unload();
}

void TrueTypeFont::loadFromMemory(array_buffer&& buffer) {
    assert(!loaded);
    const auto* fontData = buffer.data();
    const auto fontIndex = 0;
    const int fontOffset = stbtt_GetFontOffsetForIndex(fontData, fontIndex);
    assert(fontOffset >= 0 && "fontData is incorrect, or fontIndex cannot be found.");
    if (stbtt_InitFont(&info, fontData, fontOffset)) {
        source = std::move(buffer);
        loaded = true;

        int ascent, descent, lineGap;
        // Store normalized line height. The real line height is got
        // by multiplying the lineh by font size.
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
        int fh = ascent - descent;
        ascender = (float) ascent / (float) fh;
        descender = (float) descent / (float) fh;
        lineHeightMultiplier = (float) (fh + lineGap) / (float) fh;
    }
}

void TrueTypeFont::unload() {
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

bool TrueTypeFont::getGlyph(uint32_t codepoint, const FontSizeInfo& sizeInfo, Glyph& outGlyph) {
    auto it = map_.find(codepoint);
    if (it != map_.end()) {
        outGlyph = scaleGlyph(it->second, sizeInfo.rasterScale);
        return true;
    }
    auto& glyph = map_.try_emplace(codepoint).first->second;

    const auto* font = &info;
    const auto scale = stbtt_ScaleForPixelHeight(font, baseFontSize);
    const auto glyphIndex = stbtt_FindGlyphIndex(font, codepoint);
    if (!glyphIndex) {
        /* @rlyeh: glyph not found, ie, arab chars */
        return false;
    }

    int advanceWidth = 0, leftSideBearing = 0;
    int x0, y0, x1, y1;

    stbtt_GetGlyphHMetrics(font, glyphIndex, &advanceWidth, &leftSideBearing);
    stbtt_GetGlyphBitmapBox(font, glyphIndex, dpiScale * scale, dpiScale * scale, &x0, &y0, &x1, &y1);

    glyph.advanceWidth = scale * advanceWidth;
    glyph.leftSideBearing = scale * leftSideBearing;
    glyph.size = baseFontSize;

    auto glyphWidth = x1 - x0;
    auto glyphHeight = y1 - y0;

    std::vector<uint8_t> bmp;
    bmp.resize(glyphWidth * glyphHeight);

    stbtt_MakeGlyphBitmap(font, bmp.data(), glyphWidth, glyphHeight, glyphWidth, dpiScale * scale, dpiScale * scale,
                          glyphIndex);
    auto sprite = atlas.addBitmap(glyphWidth, glyphHeight, bmp);
    glyph.texture = sprite.texture;
    glyph.texCoord = sprite.texCoords;
    glyph.x0 = x0 / dpiScale;
    glyph.x1 = x1 / dpiScale;
    glyph.y0 = y0 / dpiScale;
    glyph.y1 = y1 / dpiScale;

    outGlyph = scaleGlyph(glyph, sizeInfo.rasterScale);
    return true;
}

FontSizeInfo TrueTypeFont::getSizeInfo(float size) {
    FontSizeInfo info;
    info.rasterScale = size / baseFontSize;
    info.metricsScale = 1.0f;
    info.baseFontSize = baseFontSize;
    return info;
}

void TrueTypeFont::debugDrawAtlas() {
    auto w = (float) atlas.pageWidth / dpiScale;
    auto h = (float) atlas.pageHeight / dpiScale;
    draw2d::state.set_empty_texture();
    draw2d::quad(100, 100, w, h, 0x77000000_argb);

    draw2d::state.set_texture(atlas.texture_);
    draw2d::state.set_texture_coords(0, 0, 1, 1);
    draw2d::quad(100, 100, w, h);
}

}