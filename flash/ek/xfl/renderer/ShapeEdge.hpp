#pragma once

#include <ek/math/vec.hpp>

namespace ek::xfl {

struct RenderCommand;

struct ShapeEdge {

    int fill_style_idx = -1;
    float2 p0;
    float2 c;
    float2 p1;
    bool is_quadratic = false;

    [[nodiscard]]
    RenderCommand to_command() const;

    [[nodiscard]]
    bool connects(const ShapeEdge& next) const {
        return fill_style_idx == next.fill_style_idx && equals(p1, next.p0);
    }

    static ShapeEdge curve(int style, const float2& p0, const float2& c, const float2& p1);

    static ShapeEdge line(int style, const float2& p0, const float2& p1);
};

}
