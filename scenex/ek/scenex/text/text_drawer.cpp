#include <ek/util/utf8.hpp>
#include "text_drawer.hpp"
#include "truetype_font.hpp"
#include "font.hpp"

namespace ek {

void TextDrawer::draw(const std::string& text) {
    if (!format.font) {
        return;
    }

    font = format.font;
    size = format.size;
    leading = format.leading;
    kerning = format.kerning;
    letterSpacing = format.letterSpacing;

    // render effects first
    for (int i = format.layersCount - 1; i >= 0; --i) {
        auto& layer = format.layers[i];
        if (font->getFontType() == FontType::Bitmap && layer.blurRadius > 0.0f && length_sqr(layer.offset) <= 0.1f) {
            // skip {0;0} strokes for bitmap fonts
            continue;
        }
        pass.offset = layer.offset;
        pass.blurRadius = layer.blurRadius;
        pass.blurIterations = layer.blurIterations;
        pass.filterStrength = layer.strength;
        pass.color = layer.color;
        drawPass(text);

        if(layer.glyphBounds) {
            drawGlyphBounds(text);
        }
    }
}

void TextDrawer::drawPass(const std::string& text) {
    if (font->getFontType() == FontType::Bitmap && pass.blurRadius > 0.0f && length_sqr(pass.offset) <= 0.1f) {
        return;
    }

    auto* impl = font->getImpl();
    impl->setBlur(pass.blurRadius, pass.blurIterations, pass.filterStrength);

    FontImplBase* fallbackImpl = nullptr;
    if (fallback) {
        fallbackImpl = fallback->getImpl();
        fallbackImpl->setBlur(pass.blurRadius, pass.blurIterations, pass.filterStrength);
    }

    float lineHeightMul = 0.0f;
    const float2 start = position + pass.offset;
    float2 current = start;

    draw2d::state.save_color().multiply_color(pass.color);

    const graphics::texture_t* prevTexture = nullptr;
    Glyph gdata;
    uint32_t codepoint = 0;
    uint32_t utf8decoderState = 0;
    auto* it = reinterpret_cast<const uint8_t*>(text.c_str());
    while (*it != 0) {
        if (decodeUTF8(&utf8decoderState, &codepoint, *(it++)) != 0) {
            continue;
        }
        if (codepoint == '\n' || codepoint == '\r') {
            current.x = start.x;
            current.y += size * lineHeightMul + leading;
            lineHeightMul = 0.0f;
            continue;
        }

        if (impl->getGlyph(codepoint, gdata) || (fallbackImpl && fallbackImpl->getGlyph(codepoint, gdata))) {
            if (gdata.texture) {
                if (prevTexture != gdata.texture) {
                    draw2d::state.set_texture(gdata.texture);
                    prevTexture = gdata.texture;
                }
                draw2d::state.set_texture_coords(gdata.texCoord);
                gdata.rect *= size;
                if (gdata.rotated) {
                    draw2d::quad_rotated(gdata.rect.x + current.x,
                                         gdata.rect.y + current.y,
                                         gdata.rect.width,
                                         gdata.rect.height);

                } else {
                    draw2d::quad(gdata.rect.x + current.x,
                                 gdata.rect.y + current.y,
                                 gdata.rect.width,
                                 gdata.rect.height);
                }
            }

            current.x += size * gdata.advanceWidth + letterSpacing;
            if (lineHeightMul < gdata.lineHeight) {
                lineHeightMul = gdata.lineHeight;
            }
        }
    }
    draw2d::state.restore_color();
}

void TextDrawer::drawGlyphBounds(const std::string& text) {
    auto* impl = font->getImpl();
    impl->setBlur(pass.blurRadius, pass.blurIterations, pass.filterStrength);

    FontImplBase* fallbackImpl = nullptr;
    if (fallback) {
        fallbackImpl = fallback->getImpl();
        fallbackImpl->setBlur(pass.blurRadius, pass.blurIterations, pass.filterStrength);
    }

    float lineHeightMul = 0.0f;
    const float2 start = position + pass.offset;
    float2 current = start;

    draw2d::state.set_empty_texture();

    Glyph gdata;
    uint32_t codepoint = 0;
    uint32_t utf8decoderState = 0;
    auto* it = reinterpret_cast<const uint8_t*>(text.c_str());
    while (*it != 0) {
        if (decodeUTF8(&utf8decoderState, &codepoint, *(it++)) != 0) {
            continue;
        }
        if (codepoint == '\n' || codepoint == '\r') {
            current.x = start.x;
            current.y += size * lineHeightMul + leading;
            lineHeightMul = 0.0f;
            continue;
        }

        if (impl->getGlyph(codepoint, gdata) || (fallbackImpl && fallbackImpl->getGlyph(codepoint, gdata))) {
            if (gdata.texture) {
                draw2d::strokeRect(translate(gdata.rect * size, current), 0x77FF0000_argb, 1);
            }
            current.x += size * gdata.advanceWidth + letterSpacing;
            if (lineHeightMul < gdata.lineHeight) {
                lineHeightMul = gdata.lineHeight;
            }
        }
    }
}

}