#pragma once

#include "RenderCommand.hpp"

namespace ek::xfl {

struct ShapeEdge {

    int fill_style_idx = -1;
    vec2_t p0 = {};
    vec2_t c = {};
    vec2_t p1 = {};
    bool is_quadratic = false;

    [[nodiscard]]
    RenderCommand to_command() const;

    [[nodiscard]]
    bool connects(const ShapeEdge& next) const {
        return fill_style_idx == next.fill_style_idx &&
               almost_eq_vec2(p1, next.p0, MATH_F32_EPSILON);
    }

    static ShapeEdge curve(int style, vec2_t p0, vec2_t c, vec2_t p1);

    static ShapeEdge line(int style, vec2_t p0, vec2_t p1);
};

class ShapeDecoder {
public:
    explicit ShapeDecoder(const TransformModel& transform);

    void decode(const Element& el);

    RenderCommandsBatch result() const;

    bool empty() const;

private:

    void extend(vec2_t p, float r = 0.0f);

    void read_fill_styles(const Element& el);

    void read_line_styles(const Element& el);

    void flush_commands(const Array<RenderCommand>& edges, Array<ShapeEdge>& fills);

    Array<RenderCommand> commands_{};
    Array<RenderCommand> fill_styles_{};
    Array<RenderCommand> line_styles_{};

    TransformModel transform_;
    brect_t bounds_builder_ = brect_inf();
    int total_ = 0;
};

}


