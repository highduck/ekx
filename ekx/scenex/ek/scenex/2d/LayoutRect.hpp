#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>
#include "Transform2D.hpp"

namespace ek {

struct LayoutRect {
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

    LayoutRect& enableAlignX(float relative, float absolute = 0.0f) {
        align_x = true;
        x.x = relative;
        x.y = absolute;
        return *this;
    }

    LayoutRect& enableAlignY(float relative, float absolute = 0.0f) {
        align_y = true;
        y.x = relative;
        y.y = absolute;
        return *this;
    }

    LayoutRect& fill(bool xAxis, bool yAxis) {
        fill_x = xAxis;
        fill_y = yAxis;
        return *this;
    }

    LayoutRect& setInsetsMode(bool safe) {
        doSafeInsets = safe;
        return *this;
    }

    static void updateAll();
};

rect_f find_parent_layout_rect(ecs::entity e, bool safe);


// wrapper

class layout_wrapper {
public:
    layout_wrapper(ecs::entity e) :
            e_{e},
            l_{e.get_or_create<LayoutRect>()} {
    }

    static rect_f designCanvasRect;

    layout_wrapper& aligned(float rel_x = 0.0f, float abs_x = 0.0f, float rel_y = 0.0f, float abs_y = 0.0f) {
        horizontal(rel_x, abs_x);
        vertical(rel_y, abs_y);
        return *this;
    }

    layout_wrapper& hard(float x, float y) {
        const auto pos = e_.get_or_create<Transform2D>().getPosition();
        horizontal(x, pos.x - (designCanvasRect.x + designCanvasRect.width * x));
        vertical(y, pos.y - (designCanvasRect.y + designCanvasRect.height * y));
        return *this;
    }

    layout_wrapper& hard_y(float y = 0.0f) {
        const auto posY = e_.get_or_create<Transform2D>().getPosition().y;
        vertical(y, posY - (designCanvasRect.y + designCanvasRect.height * y));
        return *this;
    }

    layout_wrapper& horizontal(float multiplier = 0.0f, float offset = 0.0f) {
        l_.enableAlignX(multiplier, offset);
        return *this;
    }

    layout_wrapper& vertical(float multiplier = 0.0f, float offset = 0.0f) {
        l_.enableAlignY(multiplier, offset);
        return *this;
    }

    layout_wrapper& fill(bool horizontal = true, bool vertical = true) {
        l_.fill(horizontal, vertical);
        return *this;
    }

    layout_wrapper& fill_extra(const rect_f& rc) {
        l_.fill_extra = rc;
        return *this;
    }

private:
    ecs::entity e_;
    LayoutRect& l_;
};
}