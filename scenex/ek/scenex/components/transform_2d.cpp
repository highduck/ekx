#include "transform_2d.hpp"

#include <ek/draw2d/drawer.hpp>

namespace ek {

void begin_transform(const transform_2d& transform) {
    const_cast<transform_2d&>(transform).updateLocalMatrix();
    draw2d::state.save_transform()
            .combine_color(transform.color_multiplier, transform.color_offset)
            .concat_matrix(transform.matrix);
}

void end_transform() {
    draw2d::state.restore_transform();
}

void transform_2d::updateLocalMatrix() {
    const auto x = position.x + origin.x;
    const auto y = position.y + origin.y;
    const auto xx = -origin.x - pivot.x;
    const auto yy = -origin.y - pivot.y;

    const auto ra = cosf(skew.y) * scale.x;
    const auto rb = sinf(skew.y) * scale.x;
    const auto rc = -sinf(skew.x) * scale.y;
    const auto rd = cosf(skew.x) * scale.y;

    matrix.a = ra;
    matrix.b = rb;
    matrix.c = rc;
    matrix.d = rd;
    matrix.tx = x + ra * xx + rc * yy;
    matrix.ty = y + rd * yy + rb * xx;
}

}