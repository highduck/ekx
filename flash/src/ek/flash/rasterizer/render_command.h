#pragma once

#include <ek/math/vec_fwd.hpp>

namespace ek::flash {

struct fill_style;
struct stroke_style;


struct render_command {

    enum class operation {
        curve_to,
        line_to,
        move_to,
        fill_end,
        fill_begin,
        line_style_reset,
        line_style_setup
    };

    operation op;
    float v[4]{};
    const fill_style* fill{};
    const stroke_style* stroke{};

    explicit render_command(operation operation)
            : op{operation} {

    }

    render_command(operation operation, const fill_style* aFill)
            : op{operation},
              fill{aFill} {

    }

    render_command(operation operation, const stroke_style* aStroke)
            : op{operation},
              stroke{aStroke} {

    }

    render_command(operation operation, const float2& p0, const float2& p1 = float2())
            : op{operation},
              v{p0.x, p0.y, p1.x, p1.y} {

    }
};

}