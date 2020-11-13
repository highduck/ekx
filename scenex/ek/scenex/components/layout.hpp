#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>

namespace ek {

struct layout_t {
    float2 x;
    float2 y;
    rect_f fill_extra;
    bool fill_x = false;
    bool fill_y = false;
    bool align_x = false;
    bool align_y = false;
    bool doSafeInsets = true;

    rect_f rect;
    rect_f safeRect;

    layout_t& enableAlignX(float relative, float absolute = 0.0f) {
        align_x = true;
        x.x = relative;
        x.y = absolute;
        return *this;
    }

    layout_t& enableAlignY(float relative, float absolute = 0.0f) {
        align_y = true;
        y.x = relative;
        y.y = absolute;
        return *this;
    }
};

rect_f find_parent_layout_rect(ecs::entity e, bool safe);

rect_f get_ancestors_rect(ecs::entity e);

void setScreenRects(ecs::entity root);

}