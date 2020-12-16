#pragma once

#include <ek/xfl/types.hpp>
#include "transform_model.hpp"
#include "shape_processor.hpp"
#include "shape_edge.hpp"

namespace ek::xfl {

class shape_decoder {
public:
    explicit shape_decoder(const transform_model& transform);

    void decode(const element_t& el);

    render_batch result() const;

    bool empty() const;

private:

    void extend(const float2& p, float r = 0.0f);

    void read_fill_styles(const element_t& el);

    void read_line_styles(const element_t& el);

    void flush_commands(const std::vector<render_command>& edges, std::vector<shape_edge>& fills);

    std::vector<render_command> commands_;
    std::vector<render_command> fill_styles_;
    std::vector<render_command> line_styles_;

    transform_model transform_;
    bounds_builder_2f bounds_builder_{};
    int total_ = 0;
};

}


