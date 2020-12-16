#pragma once

#include "ShapeProcessor.hpp"
#include "ShapeEdge.hpp"

#include <ek/xfl/types.hpp>

namespace ek::xfl {

class ShapeDecoder {
public:
    explicit ShapeDecoder(const TransformModel& transform);

    void decode(const Element& el);

    RenderCommandsBatch result() const;

    bool empty() const;

private:

    void extend(const float2& p, float r = 0.0f);

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


