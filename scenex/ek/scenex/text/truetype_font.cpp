#include "truetype_font.hpp"

#include <unordered_map>
#include <ek/app/res.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/app/device.hpp>
#include <ek/file_view.hpp>
#include <ek/imaging/fast_blur.hpp>

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
    mapByEffect[0] = std::make_unique<std::unordered_map<uint32_t, Glyph>>();
    map = mapByEffect[0].get();
}

TrueTypeFont::~TrueTypeFont() {
    unload();
}

void TrueTypeFont::loadFromMemory(array_buffer&& buffer) {
    assert(!loaded);
    if (!buffer.empty() && initFromMemory(buffer.data(), buffer.size())) {
        source = std::move(buffer);
        loaded = true;
    }
}

void TrueTypeFont::unload() {
    if (loaded) {
        source.resize(0);
        source.shrink_to_fit();

        delete info;
        info = nullptr;

        delete mappedSourceFile_;
        mappedSourceFile_ = nullptr;

        loaded = false;
    }

}

// store prerendered glyph for baseFontSize and upscaled by dpiScale
// quad scale just multiplier to get fontSize relative to baseFontSize
bool TrueTypeFont::getGlyph(uint32_t codepoint, Glyph& outGlyph) {
    assert(map != nullptr);

    auto& map_ = *map;
    auto it = map_.find(codepoint);
    if (it != map_.end()) {
        outGlyph = it->second;
        return true;
    }

    const auto glyphIndex = stbtt_FindGlyphIndex(info, codepoint);
    if (!glyphIndex) {
        /* @rlyeh: glyph not found, ie, arab chars */
        return false;
    }

    const float scale = stbtt_ScaleForPixelHeight(info, baseFontSize);
    auto& glyph = map_.try_emplace(codepoint).first->second;

    int advanceWidth = 0, leftSideBearing = 0;
    stbtt_GetGlyphHMetrics(info, glyphIndex, &advanceWidth, &leftSideBearing);
    glyph.advanceWidth = scale * static_cast<float>(advanceWidth) / baseFontSize;

    int x0, y0, x1, y1;
    stbtt_GetGlyphBitmapBox(info, glyphIndex, dpiScale * scale, dpiScale * scale, &x0, &y0, &x1, &y1);

    auto glyphWidth = x1 - x0;
    auto glyphHeight = y1 - y0;

    if (glyphWidth > 0 && glyphHeight > 0) {
        int pad = blurRadius_ * 2;

        x0 -= pad;
        y0 -= pad;
        x1 += pad;
        y1 += pad;

        auto bitmapWidth = x1 - x0;
        auto bitmapHeight = y1 - y0;

        std::vector<uint8_t> bmp;
        bmp.resize(bitmapWidth * bitmapHeight, 0);


        stbtt_MakeGlyphBitmap(info, bmp.data() + pad * bitmapWidth + pad, glyphWidth, glyphHeight, bitmapWidth,
                              dpiScale * scale, dpiScale * scale, glyphIndex);

        fastBlurA8(bmp.data(), bitmapWidth, bitmapHeight, bitmapWidth, blurRadius_, blurIterations_, strengthPower_);

        auto sprite = atlas.addBitmap(bitmapWidth, bitmapHeight, bmp);
        glyph.texture = sprite.texture;
        glyph.texCoord = sprite.texCoords;

        glyph.rect.x = x0;
        glyph.rect.y = y0;
        glyph.rect.width = bitmapWidth;
        glyph.rect.height = bitmapHeight;

        // scale to font size unit space
        glyph.rect *= 1.0f / (dpiScale * baseFontSize);
        glyph.lineHeight = lineHeightMultiplier;
    }

    outGlyph = glyph;
    return true;
}

void TrueTypeFont::debugDrawAtlas(float x, float y) {
    auto w = (float) atlas.pageWidth;
    auto h = (float) atlas.pageHeight;
    draw2d::state.set_empty_texture();
    draw2d::quad(x, y, w, h, 0x77000000_argb);

    draw2d::state.set_texture(atlas.texture_);
    draw2d::state.set_texture_coords(0, 0, 1, 1);
    draw2d::quad(x, y, w, h);
}


void TrueTypeFont::loadDeviceFont(const char* fontName) {
    assert(!loaded);

    auto path = getDeviceFontPath(fontName);
    if (!path.empty()) {
//        get_resource_content_async(path.c_str(), [this](auto buffer) {
//            loadFromMemory(std::move(buffer));
//        });
        auto* file = new FileView(path.c_str());
        if (file->size() > 0 && initFromMemory(file->data(), file->size())) {
            loaded = true;
            mappedSourceFile_ = file;
        } else {
            delete file;
        }
    }
}

bool TrueTypeFont::initFromMemory(const uint8_t* data, size_t size) {
    const auto* fontData = data;
    const auto fontIndex = 0;
    const int fontOffset = stbtt_GetFontOffsetForIndex(fontData, fontIndex);
    assert(fontOffset >= 0 && "fontData is incorrect, or fontIndex cannot be found.");
    info = new stbtt_fontinfo;
    if (stbtt_InitFont(info, fontData, fontOffset)) {
        int ascent, descent, lineGap;
        // Store normalized line height. The real line height is got
        // by multiplying the lineh by font size.
        stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);
        int fh = ascent - descent;
        ascender = (float) ascent / (float) fh;
        descender = (float) descent / (float) fh;
        lineHeightMultiplier = (float) (fh + lineGap) / (float) fh;
        return true;
    }
    delete info;
    info = nullptr;

    return false;
}

void TrueTypeFont::setBlur(float radius, int iterations, int strengthPower) {
    radius = roundf(dpiScale * radius);
    blurRadius_ = radius < 0 ? 0 : (radius > 0xFF ? 0xFF : (uint8_t) radius);
    blurIterations_ = iterations < 0 ? 0 : (iterations > 3 ? 3 : iterations);
    strengthPower_ = strengthPower < 0 ? 0 : (strengthPower > 7 ? 7 : strengthPower);
    uint32_t hashCode = (strengthPower_ << 16) | (blurIterations_ << 8) | blurRadius_;
    auto it = mapByEffect.find(hashCode);
    if (it == mapByEffect.end()) {
        mapByEffect[hashCode] = std::make_unique<std::unordered_map<uint32_t, Glyph>>();
    }
    map = mapByEffect[hashCode].get();
}


}