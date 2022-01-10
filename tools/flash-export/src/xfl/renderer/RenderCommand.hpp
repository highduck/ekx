#pragma once

#include <ek/math.h>
#include "../types.hpp"

namespace ek::xfl {

struct RenderCommand {

    enum class Operation {
        curve_to,
        line_to,
        move_to,
        fill_end,
        fill_begin,
        line_style_reset,
        line_style_setup,
        rectangle,
        oval,
        bitmap
    };

    Operation op;
    double v[8]{};
    const FillStyle* fill{};
    const StrokeStyle* stroke{};
    const BitmapData* bitmap{};

    explicit RenderCommand(Operation operation) :
            op{operation} {
    }

    RenderCommand(Operation operation, const FillStyle* aFill) :
            op{operation},
            fill{aFill} {
    }

    RenderCommand(Operation operation, const StrokeStyle* aStroke) :
            op{operation},
            stroke{aStroke} {
    }

    RenderCommand(Operation operation, const vec2_t p0, const vec2_t p1 = {}) :
            op{operation},
            v{p0.x, p0.y, p1.x, p1.y} {
    }
};

struct RenderCommandsBatch {
    TransformModel transform;
    Array<RenderCommand> commands;
    aabb2_t bounds = aabb2_empty();
    int total = 0;

    [[nodiscard]] bool empty() const {
        return commands.empty() || aabb2_is_empty(bounds) || total == 0;
    }
};

}