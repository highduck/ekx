#pragma once

#include <ecx/ecx.hpp>
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

    static void hard(entity_t e, float x, float y) {
        if(e.id) {
            const auto pos = ecs::add<Transform2D>(e).getPosition();
            ecs::add<LayoutRect>(e).aligned(
                    x, pos.x - (DesignCanvasRect.x + DesignCanvasRect.w * x),
                    y, pos.y - (DesignCanvasRect.y + DesignCanvasRect.h * y)
            );
        }
    }

    static void hardX(entity_t e, float x = 0.0f) {
        const auto pos = ecs::add<Transform2D>(e).getPosition();
        ecs::add<LayoutRect>(e).enableAlignX(
                x, pos.x - (DesignCanvasRect.x + DesignCanvasRect.w * x)
        );
    }

    static void hardY(entity_t e, float y = 0.0f) {
        const auto pos = ecs::add<Transform2D>(e).getPosition();
        ecs::add<LayoutRect>(e).enableAlignY(
                y, pos.y - (DesignCanvasRect.y + DesignCanvasRect.h * y)
        );
    }

    static void updateAll();

    inline static rect_t DesignCanvasRect = rect_01();
};



rect_t find_parent_layout_rect(entity_t e, bool safe);


// wrapper

class layout_wrapper {
public:
    layout_wrapper(entity_t e) :
            e_{e},
            l_{&ecs::add<LayoutRect>(e)} {
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
        l_->enableAlignX(multiplier, offset);
        return *this;
    }

    layout_wrapper& vertical(float multiplier = 0.0f, float offset = 0.0f) {
        l_->enableAlignY(multiplier, offset);
        return *this;
    }

    layout_wrapper& fill(bool horizontal = true, bool vertical = true) {
        l_->fill(horizontal, vertical);
        return *this;
    }

    layout_wrapper& fill_extra(const rect_t rc) {
        l_->fill_extra = rc;
        return *this;
    }

private:
    entity_t e_;
    LayoutRect* l_;
};
}