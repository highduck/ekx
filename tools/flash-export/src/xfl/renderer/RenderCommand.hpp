#pragma once

#include <ek/math/Vec.hpp>
#include "../types.hpp"
#include <ek/math/BoundsBuilder.hpp>

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

    RenderCommand(Operation operation, const Vec2f& p0, const Vec2f& p1 = Vec2f()) :
            op{operation},
            v{p0.x, p0.y, p1.x, p1.y} {
    }
};

struct RenderCommandsBatch {
    TransformModel transform;
    Array<RenderCommand> commands;
    BoundsBuilder2f bounds{};
    int total = 0;

    [[nodiscard]] bool empty() const {
        return commands.empty() || bounds.empty() || total == 0;
    }
};

}