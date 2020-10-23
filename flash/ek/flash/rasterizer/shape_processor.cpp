#include "shape_processor.hpp"
#include "shape_edge_decoder.hpp"
#include <ek/flash/doc/types.hpp>

namespace ek::flash {

void shape_processor::reset() {
    batches.clear();
    bounds = {};
}

bool shape_processor::add(const element_t& el, const transform_model& world) {
    switch (el.elementType) {
        case element_type::bitmap_item:
            return add(el.bitmap.get(), world);
        case element_type::object_oval:
        case element_type::object_rectangle:
            return addShapeObject(el, world);
        default:
            break;
    }
    shape_decoder decoder{world};
    decoder.decode(el);
    return !decoder.empty() && add(decoder.result());
}

rect_f transform(const matrix_2d& m, const rect_f& rc) {
    const float2 corners[4] = {
            m.transform(rc.x, rc.y),
            m.transform(rc.right(), rc.y),
            m.transform(rc.right(), rc.bottom()),
            m.transform(rc.x, rc.bottom())
    };
    float xMin = corners[0].x;
    float yMin = corners[0].y;
    float xMax = corners[0].x;
    float yMax = corners[0].y;
    for (int i = 1; i < 4; ++i) {
        xMin = fmin(xMin, corners[i].x);
        yMin = fmin(yMin, corners[i].y);
        xMax = fmax(xMax, corners[i].x);
        yMax = fmax(yMax, corners[i].y);
    }
    return {xMin, yMin, xMax - xMin, yMax - yMin};
}

bool shape_processor::add(const bitmap_t* bitmap, const transform_model& world) {
    render_batch batch{};
    batch.transform = world;
    render_command cmd{render_command::operation::bitmap};
    cmd.bitmap = bitmap;
    batch.commands.push_back(cmd);

    const rect_f rc{0.0f, 0.0f,
                    static_cast<float>(bitmap->width),
                    static_cast<float>(bitmap->height)};
    batch.bounds.add(rc, world.matrix);

    return add(batch);
}


bool shape_processor::add(const render_batch& batch) {
    if (!batch.commands.empty() && !batch.bounds.empty()) {
        batches.push_back(batch);
        bounds.add(batch.bounds.rect());
        return true;
    }
    return false;
}

bool shape_processor::addShapeObject(const element_t& el, const transform_model& world) {
    if (!el.shape) {
        return false;
    }
    const auto& shape = *el.shape;
    rect_f rc{shape.x, shape.y, shape.objectWidth, shape.objectHeight};
    render_command::operation op =
            el.elementType == element_type::object_rectangle ?
            render_command::operation::rectangle :
            render_command::operation::oval;

    render_command cmd{op};
    cmd.v[0] = rc.x;
    cmd.v[1] = rc.y;
    cmd.v[2] = rc.right();
    cmd.v[3] = rc.bottom();
    switch (el.elementType) {
        case element_type::object_rectangle:
            cmd.v[4] = shape.topLeftRadius;
            cmd.v[5] = shape.topRightRadius;
            cmd.v[6] = shape.bottomRightRadius;
            cmd.v[7] = shape.bottomLeftRadius;
            break;
        case element_type::object_oval:
            cmd.v[4] = shape.startAngle;
            cmd.v[5] = shape.endAngle;
            cmd.v[6] = shape.closePath ? 1 : 0;
            cmd.v[7] = shape.innerRadius / 100.0f;
            break;
        default:
            return false;
    }

    cmd.fill = !el.fills.empty() ? &el.fills[0] : nullptr;
    cmd.stroke = !el.strokes.empty() > 0 ? &el.strokes[0] : nullptr;

    render_batch batch{};
    batch.transform = world;
    batch.commands.push_back(cmd);

    const float hw = cmd.stroke ? (cmd.stroke->weight / 2.0f) : 0.0f;
    batch.bounds.add(rc, world.matrix, hw);
    return add(batch);
}

}