#pragma once

#include <utility>
#include <string>
#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/util/assets.hpp>

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

    [[nodiscard]] inline float2 anchor() const {
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

struct TextLayerEffect {
    argb32_t color = 0xFFFFFFFF_argb;
    float2 offset{};
    float blurRadius = 0.0f;
    uint8_t blurIterations = 0;
    uint8_t strength = 0;
    bool visible = true;
    bool showGlyphBounds = false;
    const char* nameID = nullptr;
};

struct TextFormat {
    asset_t <Font> font;
    float size = 16.0f;
    float leading = 0.0f;
    float letterSpacing = 0.0f;

    // not actually box alignment , but just to be shared for single line
    float2 alignment{};

    bool kerning = true;
    bool underline = false;
    bool strikethrough = false;

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
        font = asset_t<Font>{fontName};
    }

    TextFormat() = default;

    TextFormat(const char* fontName, float fontSize) :
            font{fontName},
            size{fontSize} {
    }

    void setAlignment(Alignment align) {
        alignment = align.anchor();
    }

    void addShadow(argb32_t color, float radius, int hardness = 20 /* 0..100 */, float2 offset = {}) {
        auto& layer = layers[layersCount++];
        layer.color = color;
        layer.strength = uint8_t(7 * hardness / 100);
        layer.blurIterations = 3;
        layer.blurRadius = radius;
    }
};

class text_format_t {
public:
    std::string font;
    float size;
    float lineHeight = -1.0f;
    float lineSpacing = 0.0f;
    argb32_t color{0xFFFFFFFF};
    float2 alignment;

    // TODO: remove
    bool shadow = true;
    argb32_t shadowColor{0xFF000000};
    float2 shadowOffset{1.0f, 1.0f};

    text_format_t(std::string font, float size, Alignment align = Alignment::TopLeft)
            : font{std::move(font)},
              size{size},
              alignment{align.anchor()} {
    }
};

}