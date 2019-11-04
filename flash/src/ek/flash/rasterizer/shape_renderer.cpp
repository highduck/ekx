#include "shape_renderer.h"
#include "shape_edge_decoder.h"
#include <ek/flash/doc/element_types.h>

namespace ek::flash {

void shape_renderer::reset() {
    batches.clear();
    bounds = {};
}

bool shape_renderer::add(const element_t& el, const transform_model& world) {
    if (el.elementType == element_type::bitmap_item) {
        return add(el.bitmap.get(), world);
    }
    shape_decoder decoder{world};
    decoder.decode(el);
    return !decoder.empty() && add(decoder.result());
}

bool shape_renderer::add(const bitmap_t* bitmap, const transform_model& world) {
    render_batch batch{};
    batch.transform = world;
    batch.bitmap = bitmap;
    batch.total = 1;

    const auto& m = world.matrix;
    const float2 lt{float2::zero};
    const float2 rb{static_cast<float>(bitmap->width),
                    static_cast<float>(bitmap->height)};
    batch.bounds.add(m.transform(lt));
    batch.bounds.add(m.transform(rb));

    return add(batch);
}

bool shape_renderer::add(const render_batch& batch) {
    if (batch.total > 0 && !batch.bounds.empty()) {
        batches.push_back(batch);
        bounds.add(batch.bounds.rect());
        return true;
    }
    return false;
}
}