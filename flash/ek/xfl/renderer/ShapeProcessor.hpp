#pragma once

#include <ek/math/bounds_builder.hpp>
#include <ek/xfl/types.hpp>

#include "RenderCommand.hpp"

#include <vector>

namespace ek::xfl {

struct RenderCommandsBatch {
    TransformModel transform;
    std::vector<RenderCommand> commands;
    bounds_builder_2f bounds{};
    int total = 0;

    [[nodiscard]] bool empty() const {
        return commands.empty() || bounds.empty() || total == 0;
    }
};

class ShapeProcessor {
public:
    std::vector<RenderCommandsBatch> batches;
    bounds_builder_2f bounds{};

    ShapeProcessor() = default;

    void reset();

    bool add(const Element& el, const TransformModel& world);

private:
    bool add(const RenderCommandsBatch& batch);

    bool add(const BitmapData* bitmap, const TransformModel& world);

    bool addShapeObject(const Element& el, const TransformModel& world);
};

}