#pragma once

#include <ek/math/vec.hpp>

namespace ek::xfl {

struct render_command;

struct shape_edge {

    int fill_style_idx = -1;
    float2 p0;
    float2 c;
    float2 p1;
    bool is_quadratic = false;

    [[nodiscard]]
    render_command to_command() const;

    [[nodiscard]]
    bool connects(const shape_edge& next) const {
        return fill_style_idx == next.fill_style_idx && equals(p1, next.p0);
    }

    static shape_edge curve(int style, const float2& p0, const float2& c, const float2& p1);

    static shape_edge line(int style, const float2& p0, const float2& p1);
};

}
