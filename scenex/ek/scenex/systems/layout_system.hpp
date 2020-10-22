#pragma once


#include <ecxx/ecxx.hpp>
#include <ek/scenex/components/layout.hpp>
#include <ek/scenex/components/transform_2d.hpp>

namespace ek {

void update_layout();

class layout_wrapper {
public:
    layout_wrapper(ecs::entity e)
            : e_{e},
              l_{ecs::get_or_create<layout_t>(e)} {
    }

    static rect_f designCanvasRect;

    layout_wrapper& aligned(float rel_x = 0.0f, float abs_x = 0.0f, float rel_y = 0.0f, float abs_y = 0.0f) {
        horizontal(rel_x, abs_x);
        vertical(rel_y, abs_y);
        return *this;
    }

    layout_wrapper& hard(float x, float y) {
        auto& transform = ecs::get_or_create<transform_2d>(e_);
        horizontal(x, transform.matrix.tx - (designCanvasRect.x + designCanvasRect.width * x));
        vertical(y, transform.matrix.ty - (designCanvasRect.y + designCanvasRect.height * y));
        return *this;
    }

    layout_wrapper& hard_y(float y = 0.0f) {
        auto& transform = ecs::get_or_create<transform_2d>(e_);
        vertical(y, transform.matrix.ty - (designCanvasRect.y + designCanvasRect.height * y));
        return *this;
    }

    layout_wrapper& horizontal(float multiplier = 0.0f, float offset = 0.0f) {
        l_.align_x = true;
        l_.x = {multiplier, offset};
        return *this;
    }

    layout_wrapper& vertical(float multiplier = 0.0f, float offset = 0.0f) {
        l_.align_y = true;
        l_.y = {multiplier, offset};
        return *this;
    }

    layout_wrapper& fill(bool horizontal = true, bool vertical = true) {
        l_.fill_x = horizontal;
        l_.fill_y = vertical;
        return *this;
    }

    layout_wrapper& fill_extra(const rect_f& rc) {
        l_.fill_extra = rc;
        return *this;
    }
private:
    ecs::entity e_;
    layout_t& l_;
};

}

