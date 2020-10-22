#include "shape_edge.hpp"
#include "render_command.hpp"

namespace ek::flash {

using render_op = render_command::operation;

render_command shape_edge::to_command() const {
    if (is_quadratic) {
        return {render_op::curve_to, c, p1};
    } else {
        return {render_op::line_to, p1};
    }
}

shape_edge shape_edge::curve(int style, const float2& p0, const float2& c, const float2& p1) {
    shape_edge result;
    result.fill_style_idx = style;
    result.p0 = p0;
    result.c = c;
    result.p1 = p1;
    result.is_quadratic = true;
    return result;
}

shape_edge shape_edge::line(int style, const float2& p0, const float2& p1) {
    shape_edge result;
    result.fill_style_idx = style;
    result.p0 = p0;
    result.p1 = p1;
    result.is_quadratic = false;
    return result;
}

}