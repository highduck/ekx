#pragma once

#include <ecxx/ecxx.hpp>
#include "Transform2D.hpp"

namespace ek {

struct LayoutRect {
    vec2_t x = {};
    vec2_t y = {};
    rect_t fill_extra = {};
    rect_t rect = {};
    rect_t safeRect = {};
    bool fill_x = false;
    bool fill_y = false;
    bool align_x = false;
    bool align_y = false;
    bool doSafeInsets = true;

    LayoutRect& aligned(float relativeX, float absoluteX, float relativeY, float absoluteY) {
        enableAlignX(relativeX, absoluteX);
        enableAlignY(relativeY, absoluteY);
        return *this;
    }

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

    static void hard(ecs::EntityApi e, float x, float y) {
        if(e) {
            const auto pos = e.get_or_create<Transform2D>().getPosition();
            e.get_or_create<LayoutRect>().aligned(
                    x, pos.x - (DesignCanvasRect.x + DesignCanvasRect.w * x),
                    y, pos.y - (DesignCanvasRect.y + DesignCanvasRect.h * y)
            );
        }
    }

    static void hardX(ecs::EntityApi e, float x = 0.0f) {
        const auto pos = e.get_or_create<Transform2D>().getPosition();
        e.get_or_create<LayoutRect>().enableAlignX(
                x, pos.x - (DesignCanvasRect.x + DesignCanvasRect.w * x)
        );
    }

    static void hardY(ecs::EntityApi e, float y = 0.0f) {
        const auto pos = e.get_or_create<Transform2D>().getPosition();
        e.get_or_create<LayoutRect>().enableAlignY(
                y, pos.y - (DesignCanvasRect.y + DesignCanvasRect.h * y)
        );
    }

    static void updateAll();

    inline static rect_t DesignCanvasRect = rect_01();
};

ECX_TYPE(12, LayoutRect);

rect_t find_parent_layout_rect(ecs::EntityApi e, bool safe);


// wrapper

class layout_wrapper {
public:
    layout_wrapper(ecs::EntityApi e) :
            e_{e},
            l_{e.get_or_create<LayoutRect>()} {
    }

    layout_wrapper& aligned(float rel_x = 0.0f, float abs_x = 0.0f, float rel_y = 0.0f, float abs_y = 0.0f) {
        horizontal(rel_x, abs_x);
        vertical(rel_y, abs_y);
        return *this;
    }

    layout_wrapper& hard(float x, float y) {
        LayoutRect::hard(e_, x, y);
        return *this;
    }

    layout_wrapper& hard_y(float y = 0.0f) {
        LayoutRect::hardY(e_, y);
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

    layout_wrapper& fill_extra(const rect_t rc) {
        l_.fill_extra = rc;
        return *this;
    }

private:
    ecs::EntityApi e_;
    LayoutRect& l_;
};
}