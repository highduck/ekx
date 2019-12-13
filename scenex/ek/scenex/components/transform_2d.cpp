#include "transform_2d.hpp"

#include <ek/util/locator.hpp>
#include <ek/draw2d/drawer.hpp>

namespace ek {

void begin_transform(const transform_2d& transform) {
    if (transform.user_matrix != nullptr) {
        transform.matrix = *(transform.user_matrix);
    } else {
        transform.matrix.set(transform.scale, transform.skew);
    }
    auto& drawer = resolve<drawer_t>();
    drawer.state.save_transform();
    drawer.state.combine_color(transform.color_multiplier, transform.color_offset)
            .translate(transform.origin)
            .concat_matrix(transform.matrix)
            .translate(-transform.origin);
}

void end_transform() {
    resolve<drawer_t>().state.restore_transform();
}

}
