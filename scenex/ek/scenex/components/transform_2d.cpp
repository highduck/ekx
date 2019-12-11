#include "transform_2d.hpp"

#include <ek/util/locator.hpp>
#include <ek/draw2d/drawer.hpp>

namespace ek {

void begin_transform(const transform_2d& transform) {
    if (transform.manualMatrix != nullptr) {
        transform.matrix = *(transform.manualMatrix);
    } else {
        transform.matrix.set(transform.scale, transform.skew);
    }
    auto& drawer = resolve<drawer_t>();
    drawer.save_transform();
    drawer.combine_color(transform.colorMultiplier, transform.colorOffset)
            .translate(transform.origin)
            .concat_matrix(transform.matrix)
            .translate(-transform.origin);
}

void end_transform() {
    resolve<drawer_t>().restore_transform();
}

}
