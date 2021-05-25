#include "TextEngine.hpp"
#include "TrueTypeFont.hpp"
#include "Font.hpp"
#include <ek/util/utf8.hpp>
#include <cstdarg>

namespace ek {

StaticStorage<TextEngineSharedContext> gTextEngine{};

void TextBlockInfo::Line::updateSize(float length, float height) {
    if (size.x < length) {
        size.x = length;
    }
    if (size.y < height) {
        size.y = height;
    }
}

void TextBlockInfo::Line::close(float emptyLineHeight, int end_) {
    end = end_;
    if (size.y <= 0.0f) {
        size.y = emptyLineHeight;
    }
}

void TextBlockInfo::addLine(TextBlockInfo::Line line) {
    EK_ASSERT(lines.size() < WarningLinesCount);
    if (size.x < line.size.x) {
        size.x = line.size.x;
    }
    size.y += line.size.y;
    if (!lines.empty()) {
        size.y += lines.back().leading;
    }
    lines.push_back(line);
}

void TextBlockInfo::reset() {
    size = float2::zero;
    lines.clear();
}

void TextBlockInfo::scale(float factor) {
    size *= factor;
    for (auto& line : lines) {
        line.size *= factor;
        // for first-line position
        line.ascender *= factor;
        //line.descender *= factor;
    }
}

bool TextBlockInfo::checkIsValid() const {
    int pos = 0;
    for (auto& line : lines) {
        if (line.end < line.begin) {
            EK_ASSERT(false);
            return false;
        }
        if (line.begin < pos) {
            EK_ASSERT(false);
            return false;
        }
        if (line.end < pos) {
            EK_ASSERT(false);
            return false;
        }
        pos = line.end;
    }
    return true;
}

void TextEngine::draw(const char* text) {
    if (!format.font) {
        return;
    }
    auto& info = gTextEngine.get().textBlockInfo;
    getTextSize(text, info);
    drawWithBlockInfo(text, info);
}

void TextEngine::drawWithBlockInfo(const char* text, const TextBlockInfo& info) {
    auto font = format.font;
    if (!font) {
        return;
    }
    auto alignment = format.alignment;

    draw2d::state.pushProgram("draw2d_alpha");
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
    draw2d::state.restoreProgram();
}

void TextEngine::drawLayer(const char* text, const TextLayerEffect& layer, const TextBlockInfo& info) const {
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
    int numLines = info.lines.size();

    current.x += (info.size.x - info.lines[lineIndex].size.x) * alignment.x;

    draw2d::state.save_color().scaleColor(layer.color);

    const graphics::Texture* prevTexture = nullptr;
    uint32_t prevCodepointOnLine = 0;
    Glyph gdata;
    uint32_t codepoint = 0;
    while (lineIndex < numLines) {
        const char* it = text + info.lines[lineIndex].begin;
        const auto* end = text + info.lines[lineIndex].end;
        while (it != end) {
            it = decodeUTF8(it, codepoint);
            if (font->getGlyph(codepoint, gdata)) {
                if (kerning && prevCodepointOnLine) {
                    current.x += gdata.source->getKerning(prevCodepointOnLine, codepoint) * size;
                }
                if (gdata.texture) {
                    if (prevTexture != gdata.texture) {
                        draw2d::state.setTexture(gdata.texture);
                        prevTexture = gdata.texture;
                    }
                    draw2d::state.setTextureCoords(gdata.texCoord);
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
                        draw2d::state.setEmptyTexture();
                        draw2d::strokeRect(gdata.rect, 0xFFFFFF_rgb, 1);
                        draw2d::state.setTexture(gdata.texture);
                    }
                }

                current.x += size * gdata.advanceWidth + letterSpacing;
            }
            prevCodepointOnLine = codepoint;
        }
        current.x = startX;
        current.y += info.lines[lineIndex].size.y;
        ++lineIndex;
        if (lineIndex < numLines) {
            current.x += (info.size.x - info.lines[lineIndex].size.x) * alignment.x;
            current.y += info.lines[lineIndex - 1].leading;
            prevCodepointOnLine = 0;
        }
    }

    draw2d::state.restore_color();
}

struct TextEngineUtils {
    static const char* whitespaces;
    static const char* punctuations;

    static bool inRangeASCII(uint32_t c, const char* range) {
        return c && c == (c & 0x7Fu) && strchr(range, static_cast<char>(c & 0x7Fu));
    }

    static const char* skip(const char* it, const char* range) {
        uint32_t c = 0;
        const auto* prev = it;
        it = decodeUTF8(it, c);
        while (inRangeASCII(c, range)) {
            prev = it;
            it = decodeUTF8(it, c);
        }
        return prev;
    }

    static bool isWhitespace(uint32_t c) {
        return inRangeASCII(c, whitespaces);
    }

    static bool isPunctuation(uint32_t c) {
        return inRangeASCII(c, punctuations);
    }

    static const char* skipWhitespaces(const char* it) {
        return skip(it, whitespaces);
    }
};

const char* TextEngineUtils::whitespaces = " \n\t";
const char* TextEngineUtils::punctuations = ".,!?:";

void TextEngine::getTextSize(const char* text, TextBlockInfo& info) const {
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
    const char* it = text;
    const char* prev = text;
    const char* lastWrapToPosition = nullptr;
    TextBlockInfo::Line lastWrapLine;

    uint32_t prevCodepointOnLine = 0;
    uint32_t codepoint = 0;
    TextBlockInfo::Line line;
    it = decodeUTF8(it, codepoint);
    while (codepoint) {
        if (codepoint == '\n') {
            line.close(size, static_cast<int>(prev - text));
            info.addLine(line);
            line = {};
            line.begin = static_cast<int>(it - text);
            x = 0.0f;

            lastWrapLine = {};
            // next char
            prevCodepointOnLine = 0;
            prev = it;
            it = decodeUTF8(it, codepoint);
            continue;
        }
        // wordwrap
        if (format.wordWrap) {
            if (TextEngineUtils::isPunctuation(prevCodepointOnLine)) {
                lastWrapToPosition = TextEngineUtils::skipWhitespaces(prev);
                lastWrapLine = line;
                lastWrapLine.close(size, static_cast<int>(prev - text));
            } else if (TextEngineUtils::isWhitespace(codepoint)) {
                lastWrapToPosition = TextEngineUtils::skipWhitespaces(it);
                lastWrapLine = line;
                lastWrapLine.close(size, static_cast<int>(prev - text));
            }
        }
        if (font->getGlyphMetrics(codepoint, metrics)) {
            const auto kern = (kerning && prevCodepointOnLine) ?
                              metrics.source->getKerning(prevCodepointOnLine, codepoint) * size :
                              0.0f;
            auto right = x + kern + size * fmax(metrics.rect.right(), metrics.advanceWidth);
            if (format.wordWrap && right > maxWidth && maxWidth > 0) {
                if (lastWrapLine.end != 0) {
                    info.addLine(lastWrapLine);
                    line = {};
                    it = lastWrapToPosition;
                    line.begin = static_cast<int>(it - text);
                    x = 0.0f;
                    lastWrapLine = {};
                    prevCodepointOnLine = 0;
                    prev = it;
                    it = decodeUTF8(it, codepoint);
                    continue;
                }
                    // at least one symbol added to line
                else if (it > text + line.begin && format.allowLetterWrap) {
                    line.close(size, static_cast<int>(prev - text));
                    info.addLine(line);
                    line = {};
                    lastWrapLine = {};
                    line.begin = static_cast<int>(prev - text);
                    x = 0.0f;
                    prevCodepointOnLine = 0;
                    // use current codepoint
                    continue;
                }
            }
            if (line.ascender < size * metrics.ascender) {
                line.ascender = size * metrics.ascender;
            }
            if (line.descender < size * metrics.descender) {
                line.descender = size * metrics.descender;
            }
            line.leading = leading;
            line.updateSize(right, size * metrics.lineHeight);
            x += kern + size * metrics.advanceWidth + letterSpacing;
        }
        prevCodepointOnLine = codepoint;
        prev = it;
        it = decodeUTF8(it, codepoint);
    }
    line.close(size, static_cast<int>(prev - text));
    info.addLine(line);

    EK_ASSERT(info.checkIsValid() == true);
}

void TextEngine::drawFormat(const char* fmt, ...) {
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