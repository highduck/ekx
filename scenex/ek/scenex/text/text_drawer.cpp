#include <ek/util/utf8.hpp>
#include "text_drawer.hpp"
#include "truetype_font.hpp"
#include "font.hpp"

namespace ek {

void TextDrawer::draw(const std::string& text) const {
    if (font.empty()) {
        return;
    }

    auto* impl = const_cast<FontImplBase*>(font->getImpl());

    FontImplBase* nativeImpl = nullptr;
    if (nativeFont) {
        nativeImpl = const_cast<FontImplBase*>(nativeFont->getImpl());
    }

    float lineHeightMul = 0.0f;
    float2 current = position;
    float2 start = position;

    draw2d::state.save_color()
            .multiply_color(textColor);
    // var vertexColor = drawer.calcVertexColorMultiplier(color);

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
            current.y += fontSize * lineHeightMul + lineSpacing;
            lineHeightMul = 0.0f;
            continue;
        }

        if (impl->getGlyph(codepoint, gdata) || (nativeImpl && nativeImpl->getGlyph(codepoint, gdata))) {
            if (gdata.texture) {
                if (prevTexture != gdata.texture) {
                    draw2d::state.set_texture(gdata.texture);
                    prevTexture = gdata.texture;
                }
                draw2d::state.set_texture_coords(gdata.texCoord);
                gdata.rect *= fontSize;
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

            current.x += fontSize * gdata.advanceWidth;
            if(lineHeightMul < gdata.lineHeight) {
                lineHeightMul = gdata.lineHeight;
            }
        }
    }
    draw2d::state.restore_color();
}

void TextDrawer::setBlur(float radius, int iterations, int strengthPower) {
    if (font) {
        font->getImpl()->setBlur(radius, iterations, strengthPower);
    }
    if (nativeFont) {
        nativeFont->getImpl()->setBlur(radius, iterations, strengthPower);
    }
}

}