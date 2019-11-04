#pragma once

#include <utility>
#include <string>
#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>
#include <scenex/config.h>

namespace scenex {

struct align_t {
    align_t(uint8_t flags)
            : flags_{flags} {
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

class text_format_t {
public:
    std::string font;
    float size;
    float lineHeight;
    float lineSpacing = 0.0f;
    argb32_t color{0xFFFFFFFF};
    float2 alignment;

    // TODO: remove
    bool shadow = true;
    argb32_t shadowColor{0xFF000000};
    float2 shadowOffset{1.0f, 1.0f};

    text_format_t(std::string font, float size, align_t align = align_t::TopLeft)
            : font{std::move(font)},
              size{size},
              lineHeight{size},
              alignment{align.anchor()} {
    }
};

}