#pragma once

#include "RenderCommand.hpp"

namespace ek::xfl {

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

class ShapeDecoder {
public:
    explicit ShapeDecoder(const TransformModel& transform);

    void decode(const Element& el);

    RenderCommandsBatch result() const;

    bool empty() const;

private:

    void extend(float2 p, float r = 0.0f);

    void read_fill_styles(const Element& el);

    void read_line_styles(const Element& el);

    void flush_commands(const std::vector<RenderCommand>& edges, std::vector<ShapeEdge>& fills);

    std::vector<RenderCommand> commands_;
    std::vector<RenderCommand> fill_styles_;
    std::vector<RenderCommand> line_styles_;

    TransformModel transform_;
    bounds_builder_2f bounds_builder_{};
    int total_ = 0;
};

}


