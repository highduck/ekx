#pragma once

namespace ek::flash {

enum class blend_mode_t {
    last, // default
    normal, // default
    layer,
    multiply,
    screen,
    lighten,
    darken,
    difference,
    add,
    subtract,
    invert,
    alpha,
    erase,
    overlay,
    hardlight
};

}