#include <ek/util/utf8.hpp>
#include "text_drawer.hpp"
#include "truetype_font.hpp"
#include "font.hpp"

namespace ek {

void TextBlockInfo::pushLine(float emptyLineHeight) {
    assert(numLines < 128);
    if (maxLength < lineLength[numLines]) {
        maxLength = lineLength[numLines];
    }
    if (lineHeight[numLines] <= 0.0f) {
        lineHeight[numLines] = emptyLineHeight;
    }
    totalHeight += lineHeight[numLines];
    ++numLines;
    lineLength[numLines] = 0.0f;
    lineHeight[numLines] = 0.0f;
}

void TextBlockInfo::reset() {
    bounds = {};
    maxLength = 0.0f;
    totalHeight = 0.0f;
    numLines = 0;
    lineLength[numLines] = 0.0f;
    lineHeight[numLines] = 0.0f;
}

void TextBlockInfo::updateLineLength(float length) {
    if (lineLength[numLines] < length) {
        lineLength[numLines] = length;
    }
}

void TextBlockInfo::updateLineHeight(float height) {
    if (lineHeight[numLines] < height) {
        lineHeight[numLines] = height;
    }
}

TextBlockInfo TextDrawer::sharedTextBlockInfo{};

void TextDrawer::draw(const char* text) {
    if (!format.font) {
        return;
    }
    font = format.font;
    size = format.size;
    leading = format.leading;
    kerning = format.kerning;
    letterSpacing = format.letterSpacing;
    alignment = format.alignment;

    auto& info = sharedTextBlockInfo;
    getTextSize(text, info);

    // render effects first
    for (int i = format.layersCount - 1; i >= 0; --i) {
        auto& layer = format.layers[i];
        if (!layer.visible) {
            continue;
        }
        if (font->getFontType() == FontType::Bitmap && layer.blurRadius > 0.0f && length_sqr(layer.offset) <= 0.1f) {
            // skip {0;0} strokes for bitmap fonts
            continue;
        }
        pass.offset = layer.offset;
        pass.blurRadius = layer.blurRadius;
        pass.blurIterations = layer.blurIterations;
        pass.filterStrength = layer.strength;
        pass.color = layer.color;
        pass.showGlyphBounds = layer.showGlyphBounds;
        drawPass(text, info);
    }
}

void TextDrawer::drawPass(const char* text, const TextBlockInfo& info) {
    font->setBlur(pass.blurRadius, pass.blurIterations, pass.filterStrength);

    const float2 start = rect.position + pass.offset;
    int lineIndex = 0;
    float2 current = start;
    current.x += (info.maxLength - info.lineLength[lineIndex]) * alignment.x;

    draw2d::state.save_color().multiply_color(pass.color);

    const graphics::texture_t* prevTexture = nullptr;
    uint32_t prevCodepointOnLine = 0;
    Glyph gdata;
    UTF8Decoder decoder{text};
    uint32_t codepoint = 0;
    while (decoder.decode(codepoint)) {
        if (codepoint == '\n' || codepoint == '\r') {
            current.x = start.x;
            current.y += info.lineHeight[lineIndex];
            ++lineIndex;
            current.x += (info.maxLength - info.lineLength[lineIndex]) * alignment.x;
            prevCodepointOnLine = 0;
            continue;
        }

        if (font->getGlyph(codepoint, gdata)) {
            if (kerning && prevCodepointOnLine) {
                current.x += font->getKerning(prevCodepointOnLine, codepoint) * size;
            }
            if (gdata.texture) {
                if (prevTexture != gdata.texture) {
                    draw2d::state.set_texture(gdata.texture);
                    prevTexture = gdata.texture;
                }
                draw2d::state.set_texture_coords(gdata.texCoord);
                //gdata.rect.x += gdata.bearingX;
                gdata.rect = translate(gdata.rect * size, current);
                if (!gdata.rotated) {
                    draw2d::quad(gdata.rect.x,
                                 gdata.rect.y,
                                 gdata.rect.width,
                                 gdata.rect.height);
                } else {
                    draw2d::quad_rotated(gdata.rect.x,
                                         gdata.rect.y,
                                         gdata.rect.width,
                                         gdata.rect.height);
                }
                // only for DEV mode
                if (pass.showGlyphBounds) {
                    draw2d::state.set_empty_texture();
                    draw2d::strokeRect(gdata.rect, 0xFFFFFF_rgb, 1);
                    draw2d::state.set_texture(gdata.texture);
                }
            }

            current.x += size * gdata.advanceWidth + letterSpacing;
        }
        prevCodepointOnLine = codepoint;
    }

    draw2d::state.restore_color();
}

void TextDrawer::getTextSize(const char* text, TextBlockInfo& info) {
    info.reset();

    if (!format.font) {
        return;
    }

    font = format.font;
    size = format.size;
    leading = format.leading;
    kerning = format.kerning;
    letterSpacing = format.letterSpacing;
    alignment = format.alignment;

    float x;
    Glyph metrics;
    UTF8Decoder decoder{text};
    uint32_t codepoint = 0;
    while (decoder.decode(codepoint)) {
        if (codepoint == '\n' || codepoint == '\r') {
            info.pushLine(size + leading);
            x = 0.0f;
            continue;
        }

        if (font->getGlyphMetrics(codepoint, metrics)) {
            //metrics.rect.x += metrics.bearingX;
            info.updateLineLength(x + size * fmax(metrics.rect.right(), metrics.advanceWidth));
            info.updateLineHeight(size * metrics.lineHeight + leading);
            x += size * metrics.advanceWidth + letterSpacing;
        }
    }
    info.pushLine(size);
    info.bounds = {{0.0f,           -info.lineHeight[0]},
                   {info.maxLength, info.totalHeight}};
}

}