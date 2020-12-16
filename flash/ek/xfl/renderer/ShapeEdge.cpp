#include "ShapeEdge.hpp"
#include "RenderCommand.hpp"

namespace ek::xfl {

using Op = RenderCommand::Operation;

RenderCommand ShapeEdge::to_command() const {
    if (is_quadratic) {
        return {Op::curve_to, c, p1};
    } else {
        return {Op::line_to, p1};
    }
}

ShapeEdge ShapeEdge::curve(int style, const float2& p0, const float2& c, const float2& p1) {
    ShapeEdge result;
    result.fill_style_idx = style;
    result.p0 = p0;
    result.c = c;
    result.p1 = p1;
    result.is_quadratic = true;
    return result;
}

ShapeEdge ShapeEdge::line(int style, const float2& p0, const float2& p1) {
    ShapeEdge result;
    result.fill_style_idx = style;
    result.p0 = p0;
    result.p1 = p1;
    result.is_quadratic = false;
    return result;
}

}