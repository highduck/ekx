#pragma once

#include <ek/math/Vec.hpp>
#include <ek/math/Color32.hpp>
#include <ek/util/Res.hpp>
#include "Font.hpp"

namespace ek {

struct Alignment {
    Alignment(uint8_t flags) :
            flags_{flags} {
    }

    [[nodiscard]] inline bool is_top() const {
        return (flags_ & Top) != 0;
    }

    [[nodiscard]] inline bool is_bottom() const {
        return (flags_ & Bottom) != 0;
    }

    [[nodiscard]] inline bool is_left() const {
        return (flags_ & Left) != 0;
    }

    [[nodiscard]] inline bool is_right() const {
        return (flags_ & Right) != 0;
    }

    [[nodiscard]] inline Vec2f anchor() const {
        return {
                is_right() ? 1.0f : (is_left() ? 0.0f : 0.5f),
                is_bottom() ? 1.0f : (is_top() ? 0.0f : 0.5f)
        };
    }

    enum {
        Left = 1u,
        Top = 2u,
        Center = 4u,
        Right = 8u,
        Bottom = 16u,
        TopLeft = Top | Left,
        CenterBottom = Center | Bottom
    };

private:
    uint8_t flags_ = 0u;
};

class Font;

enum class TextLayerType {
    Text,
    Stroke1,
    Stroke2,
    Shadow
};

// type + color + offset = 16
// blur + effects = 8
// Total: 24 bytes
struct TextLayerEffect {
    TextLayerType type = TextLayerType::Text;
    argb32_t color = 0xFFFFFFFF_argb;
    Vec2f offset{};
    float blurRadius = 0.0f;
    uint8_t blurIterations = 0;
    uint8_t strength = 0;
    bool visible = true;
    bool showGlyphBounds = false;
};

// 8 + 4 + 4 + 4 + 8 + 4(flags) = 32
// layers = 24 bytes * 4 (LayersMax) = 96
// count / indx = 8
// Total = 136 bytes
struct TextFormat {
    Res <Font> font;
    float size = 16.0f;
    float leading = 0.0f;
    float letterSpacing = 0.0f;

    // not actually box alignment , but just to be shared for single line
    Vec2f alignment{};

    bool kerning = true;
    bool underline = false;
    bool strikethrough = false;
    bool wordWrap = false;
    bool allowLetterWrap = true;

    inline static const int LayersMax = 4;

    TextLayerEffect layers[LayersMax]{};
    int layersCount = 1;
    int textLayerIndex = 0;

    void setTextColor(argb32_t color) {
        layers[textLayerIndex].color = color;
    }

    [[nodiscard]] argb32_t getTextColor() const {
        return layers[textLayerIndex].color;
    }

    void setFontName(const char* fontName) {
        font.setID(fontName);
    }

    TextFormat() = default;

    TextFormat(const char* fontName, float fontSize) :
            font{fontName},
            size{fontSize} {
    }

    void setAlignment(Alignment align) {
        alignment = align.anchor();
    }

    void addShadow(argb32_t color, float radius, Vec2f offset = Vec2f::zero, float hardness = 0.2f /* 0..1 */) {
        auto& layer = layers[layersCount++];
        layer.type = TextLayerType::Shadow;
        layer.color = color;
        layer.strength = uint8_t(7 * hardness);
        layer.blurIterations = 3;
        layer.blurRadius = radius;
        layer.offset = offset;
    }
};

}