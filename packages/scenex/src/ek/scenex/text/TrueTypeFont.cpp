#include "TrueTypeFont.hpp"

#include <ek/local_res.h>
#include <ek/app.h>
#include <ek/assert.h>
#include <ek/bitmap.h>

#ifndef STB_TRUETYPE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

#include <stb/stb_truetype.h>

#pragma clang diagnostic pop

#endif

#include <ek/scenex/2d/DynamicAtlas.hpp>

namespace ek {

TrueTypeFont::TrueTypeFont(float dpiScale_, float fontSize, string_hash_t dynamicAtlasName) :
        FontImplBase(FontType::TrueType),
        baseFontSize{fontSize},
        dpiScale{dpiScale_},
        atlas{R_DYNAMIC_ATLAS(dynamicAtlasName)} {
}

TrueTypeFont::~TrueTypeFont() {
    unload();
}

void TrueTypeFont::loadFromMemory(ek_local_res* lr) {
    EK_ASSERT(!loaded_);
    if (lr && lr->buffer && initFromMemory(lr->buffer, lr->length)) {
        source = *lr;
        loaded_ = true;
    }
}

void TrueTypeFont::unload() {
    if (loaded_) {
        ek_local_res_close(&source);

        delete info;
        info = nullptr;

        loaded_ = false;
    }
}

// store pre-rendered glyph for baseFontSize and upscaled by dpiScale
// quad scale just multiplier to get fontSize relative to baseFontSize
bool TrueTypeFont::getGlyph(uint32_t codepoint, Glyph& outGlyph) {
    if (!loaded_ || !atlas) {
        return false;
    }

    dynamic_atlas_ptr atlas_instance = REF_RESOLVE(res_dynamic_atlas, atlas);
    if (!atlas_instance) {
        resetGlyphs();
        // not ready to fill dynamic atlas :(
        return false;
    } else if (atlasVersion != atlas_instance->version) {
        resetGlyphs();
    }

    const uint64_t hash = effectKeyBits | codepoint;
    const auto* it = map.tryGet(hash);
    if (it) {
        outGlyph = *it;
        return true;
    }

    const auto glyphIndex = stbtt_FindGlyphIndex(info, codepoint);
    if (!glyphIndex) {
        /* @rlyeh: glyph not found, ie, arab chars */
        return false;
    }

    const float scale = stbtt_ScaleForPixelHeight(info, baseFontSize);
    map.set(hash, {});
    auto& glyph = (Glyph&) *map.tryGet(hash);
    glyph.source = this;

    int advanceWidth = 0, leftSideBearing = 0;
    stbtt_GetGlyphHMetrics(info, glyphIndex, &advanceWidth, &leftSideBearing);
    glyph.advanceWidth = scale * static_cast<float>(advanceWidth) / baseFontSize;
    glyph.bearingX = scale * static_cast<float>(leftSideBearing) / baseFontSize;

    int x0, y0, x1, y1;
    stbtt_GetGlyphBitmapBox(info, glyphIndex, dpiScale * scale, dpiScale * scale, &x0, &y0, &x1, &y1);

    auto glyphWidth = x1 - x0;
    auto glyphHeight = y1 - y0;

    if (glyphWidth > 0 && glyphHeight > 0) {
        int pad = blurRadius_;

        x0 -= pad;
        y0 -= pad;
        x1 += pad;
        y1 += pad;

        auto bitmapWidth = x1 - x0;
        auto bitmapHeight = y1 - y0;

        size_t bmpSize = bitmapWidth * bitmapHeight;
        uint8_t bmp[512 * 512];
        EK_ASSERT(bmpSize < 512 * 512);
        memset(bmp, 0, bmpSize);
        //uint8_t* bmp = (uint8_t*)calloc(1, bmpSize);

        stbtt_MakeGlyphBitmap(info, bmp + pad * bitmapWidth + pad, glyphWidth, glyphHeight, bitmapWidth,
                              dpiScale * scale, dpiScale * scale, glyphIndex);

        ek_bitmap_blur_gray(bmp, bitmapWidth, bitmapHeight, bitmapWidth, blurRadius_, blurIterations_,
                            strengthPower_);

        auto sprite = atlas_instance->addBitmap(bitmapWidth, bitmapHeight, bmp, bmpSize);
        //free(bmp);

        glyph.image = sprite.image;
        glyph.texCoord = sprite.texCoords;

        glyph.rect.x = x0;
        glyph.rect.y = y0;
        glyph.rect.w = bitmapWidth;
        glyph.rect.h = bitmapHeight;

        // scale to font size unit space
        glyph.rect = rect_scale_f(glyph.rect, 1.0f / (dpiScale * baseFontSize));
        glyph.lineHeight = lineHeightMultiplier;
        glyph.ascender = ascender;
        glyph.descender = descender;
    }

    outGlyph = glyph;
    return true;
}

bool TrueTypeFont::getGlyphMetrics(uint32_t codepoint, Glyph& outGlyph) {
    if (!loaded_) {
        return false;
    }

    auto* it = map.tryGet(effectKeyBits | codepoint);
    if (it) {
        outGlyph = *it;
        return true;
    }

    const auto glyphIndex = stbtt_FindGlyphIndex(info, codepoint);
    if (!glyphIndex) {
        /* @rlyeh: glyph not found, ie, arab chars */
        return false;
    }

    const float scale = stbtt_ScaleForPixelHeight(info, baseFontSize);
    int advanceWidth = 0, leftSideBearing = 0;
    stbtt_GetGlyphHMetrics(info, glyphIndex, &advanceWidth, &leftSideBearing);
    outGlyph.advanceWidth = scale * static_cast<float>(advanceWidth) / baseFontSize;
    outGlyph.bearingX = scale * static_cast<float>(leftSideBearing) / baseFontSize;
    outGlyph.lineHeight = lineHeightMultiplier;
    outGlyph.ascender = ascender;
    outGlyph.descender = descender;

    int x0, y0, x1, y1;
    stbtt_GetGlyphBitmapBox(info, glyphIndex, dpiScale * scale, dpiScale * scale, &x0, &y0, &x1, &y1);
    outGlyph.rect.x = x0;
    outGlyph.rect.y = y0;
    outGlyph.rect.w = x1 - x0;
    outGlyph.rect.h = y1 - y0;
    outGlyph.rect = rect_scale_f(outGlyph.rect, 1.0f / (dpiScale * baseFontSize));

    outGlyph.source = this;
    return true;
}

void TrueTypeFont::loadDeviceFont(const char* fontName) {
    EK_ASSERT(!loaded_);

    char fontPath[1024];
    if (0 == ek_app_font_path(fontPath, sizeof(fontPath), fontName) &&
        *fontPath) {
        ek_local_res_load(fontPath, [](ek_local_res* lr) {
            TrueTypeFont* this_ = (TrueTypeFont*) lr->userdata;
            this_->loadFromMemory(lr);
        }, this);
    }
}

bool TrueTypeFont::initFromMemory(const uint8_t* data, size_t size) {
    const auto* fontData = data;
    const auto fontIndex = 0;
    const int fontOffset = stbtt_GetFontOffsetForIndex(fontData, fontIndex);
    EK_ASSERT(fontOffset >= 0 && "fontData is incorrect, or fontIndex cannot be found.");
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
    if (radius > 0.0f && iterations > 0) {
        radius = roundf(dpiScale * radius);
        blurRadius_ = radius < 0 ? 0 : (radius > 0xFF ? 0xFF : (uint8_t) radius);
        blurIterations_ = iterations < 0 ? 0 : (iterations > 3 ? 3 : iterations);
        strengthPower_ = strengthPower < 0 ? 0 : (strengthPower > 7 ? 7 : strengthPower);
        effectKeyBits = (strengthPower_ << 16) | (blurIterations_ << 8) | blurRadius_;
        effectKeyBits = effectKeyBits << 32u;
    } else {
        blurRadius_ = 0;
        blurIterations_ = 0;
        strengthPower_ = 0;
        effectKeyBits = 0;
    }
}

float TrueTypeFont::getKerning(uint32_t codepoint1, uint32_t codepoint2) {
    if (loaded_ && info->kern) {
        int kern = stbtt_GetCodepointKernAdvance(info, codepoint1, codepoint2);
        if (kern != 0) {
            float scale = stbtt_ScaleForPixelHeight(info, baseFontSize);
            return static_cast<float>(kern) * scale / baseFontSize;
        }
    }
    return 0.0f;
}

void TrueTypeFont::resetGlyphs() {
    map.clear();
    if (atlas) {
        dynamic_atlas_ptr atlas_instance = REF_RESOLVE(res_dynamic_atlas, atlas);
        if (atlas_instance) {
            atlasVersion = atlas_instance->version;
        }
    }
}

}