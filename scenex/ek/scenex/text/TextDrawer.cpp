#include <ek/util/utf8.hpp>
#include "TextDrawer.hpp"
#include "TrueTypeFont.hpp"
#include "Font.hpp"

namespace ek {

TextDrawer TextDrawer::shared{};

void TextBlockInfo::pushLine(float emptyLineHeight) {
    assert(numLines < 128);
    if (size.x < line[numLines].x) {
        size.x = line[numLines].x;
    }
    if (line[numLines].y <= 0.0f) {
        line[numLines].y = emptyLineHeight;
    }
    size.y += line[numLines].y;
    ++numLines;
    line[numLines] = float2::zero;
}

void TextBlockInfo::reset() {
    size = float2::zero;
    line[0] = float2::zero;
    numLines = 0;
}

void TextBlockInfo::updateLine(float length, float height) {
    if (line[numLines].x < length) {
        line[numLines].x = length;
    }
    if (line[numLines].y < height) {
        line[numLines].y = height;
    }
}

TextBlockInfo TextDrawer::sharedTextBlockInfo{};

void TextDrawer::draw(const char *text) {
    if (!format.font) {
        return;
    }
    auto& info = sharedTextBlockInfo;
    getTextSize(text, info);
    drawWithBlockInfo(text, info);
}

void TextDrawer::drawWithBlockInfo(const char *text, const TextBlockInfo& info) {
    auto font = format.font;
    if (!font) {
        return;
    }
    auto alignment = format.alignment;

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
        drawLayer(text, layer, info);
    }
}

void TextDrawer::drawLayer(const char *text, const TextLayerEffect& layer, const TextBlockInfo& info) const {
    auto font = format.font;
    if (!font) {
        return;
    }
    auto alignment = format.alignment;
    auto size = format.size;
    auto letterSpacing = format.letterSpacing;
    auto kerning = format.kerning;

    font->setBlur(layer.blurRadius, layer.blurIterations, layer.strength);

    float2 current = position + layer.offset;
    const float startX = current.x;
    int lineIndex = 0;

    current.x += (info.size.x - info.line[lineIndex].x) * alignment.x;

    draw2d::state.save_color().scaleColor(layer.color);

    const graphics::texture_t *prevTexture = nullptr;
    uint32_t prevCodepointOnLine = 0;
    Glyph gdata;
    UTF8Decoder decoder{text};
    uint32_t codepoint = 0;
    while (decoder.decode(codepoint)) {
        if (codepoint == '\n') {
            current.x = startX;
            current.y += info.line[lineIndex].y;
            ++lineIndex;
            current.x += (info.size.x - info.line[lineIndex].x) * alignment.x;
            prevCodepointOnLine = 0;
            continue;
        }

        if (font->getGlyph(codepoint, gdata)) {
            if (kerning && prevCodepointOnLine) {
                current.x += gdata.source->getKerning(prevCodepointOnLine, codepoint) * size;
            }
            if (gdata.texture) {
                if (prevTexture != gdata.texture) {
                    draw2d::state.set_texture(gdata.texture);
                    prevTexture = gdata.texture;
                }
                draw2d::state.set_texture_coords(gdata.texCoord);
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
                if (layer.showGlyphBounds) {
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

void TextDrawer::getTextSize(const char *text, TextBlockInfo& info) const {
    info.reset();

    if (!format.font) {
        return;
    }

    auto font = format.font;
    auto size = format.size;
    auto leading = format.leading;
    auto kerning = format.kerning;
    auto letterSpacing = format.letterSpacing;
    auto alignment = format.alignment;

    float x = 0.0f;
    Glyph metrics;
    UTF8Decoder decoder{text};
    uint32_t prevCodepointOnLine = 0;
    uint32_t codepoint = 0;
    while (decoder.decode(codepoint)) {
        if (codepoint == '\n') {
            info.pushLine(size + leading);
            x = 0.0f;
            continue;
        }

        if (font->getGlyphMetrics(codepoint, metrics)) {
            if (kerning && prevCodepointOnLine) {
                x += metrics.source->getKerning(prevCodepointOnLine, codepoint) * size;
            }
            info.updateLine(
                    x + size * fmax(metrics.rect.right(), metrics.advanceWidth),
                    size * metrics.lineHeight + leading
            );
            x += size * metrics.advanceWidth + letterSpacing;
        }
        prevCodepointOnLine = codepoint;
    }
    info.pushLine(size);
}

void TextDrawer::drawFormat(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    // TODO: TextBuffer with dynamic growing memory for text?
    const int bufferSize = 1024;
    static char BUFFER[1024];
    int w = vsnprintf(BUFFER, bufferSize, fmt, args);
//    if (buf == NULL)
//        return w;
    if (w == -1 || w >= (int) bufferSize) {
        w = (int) bufferSize - 1;
    }
    BUFFER[w] = 0;
    va_end(args);

    draw(BUFFER);
}

}