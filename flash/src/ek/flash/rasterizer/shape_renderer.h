#pragma once

#include <ek/math/bounds_builder.hpp>

#include "render_command.h"
#include "transform_model.h"

#include <vector>

namespace ek::flash {

struct element_t;
struct shape_edge;
struct bitmap_t;

struct render_batch {
    transform_model transform;
    std::vector<render_command> commands;
    bounds_builder_2f bounds{};
    const bitmap_t* bitmap = nullptr;
    int total = 0;
};

class shape_renderer {
public:
    std::vector<render_batch> batches;
    bounds_builder_2f bounds{};

    shape_renderer() = default;

    void reset();

    bool add(const render_batch& batch);

    bool add(const element_t& el, const transform_model& world);

    bool add(const bitmap_t* bitmap, const transform_model& world);
};

}