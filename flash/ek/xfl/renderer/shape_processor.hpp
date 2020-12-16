#pragma once

#include <ek/math/bounds_builder.hpp>

#include "render_command.hpp"
#include "transform_model.hpp"

#include <vector>

namespace ek::xfl {

struct element_t;
struct shape_edge;
struct BitmapData;

struct render_batch {
    transform_model transform;
    std::vector<render_command> commands;
    bounds_builder_2f bounds{};
    int total = 0;
};

class shape_processor {
public:
    std::vector<render_batch> batches;
    bounds_builder_2f bounds{};

    shape_processor() = default;

    void reset();

    bool add(const element_t& el, const transform_model& world);

private:
    bool add(const render_batch& batch);

    bool add(const BitmapData* bitmap, const transform_model& world);

    bool addShapeObject(const element_t& el, const transform_model& world);
};

}