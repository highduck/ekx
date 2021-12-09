#pragma once

#include "RenderCommand.hpp"

namespace ek::xfl {

struct ShapeEdge {

    int fill_style_idx = -1;
    Vec2f p0;
    Vec2f c;
    Vec2f p1;
    bool is_quadratic = false;

    [[nodiscard]]
    RenderCommand to_command() const;

    [[nodiscard]]
    bool connects(const ShapeEdge& next) const {
        return fill_style_idx == next.fill_style_idx && equals(p1, next.p0);
    }

    static ShapeEdge curve(int style, const Vec2f& p0, const Vec2f& c, const Vec2f& p1);

    static ShapeEdge line(int style, const Vec2f& p0, const Vec2f& p1);
};

class ShapeDecoder {
public:
    explicit ShapeDecoder(const TransformModel& transform);

    void decode(const Element& el);

    RenderCommandsBatch result() const;

    bool empty() const;

private:

    void extend(Vec2f p, float r = 0.0f);

    void read_fill_styles(const Element& el);

    void read_line_styles(const Element& el);

    void flush_commands(const Array<RenderCommand>& edges, Array<ShapeEdge>& fills);

    Array<RenderCommand> commands_{};
    Array<RenderCommand> fill_styles_{};
    Array<RenderCommand> line_styles_{};

    TransformModel transform_;
    BoundsBuilder2f bounds_builder_{};
    int total_ = 0;
};

}


