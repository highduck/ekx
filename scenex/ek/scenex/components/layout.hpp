#pragma once

#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>

namespace ek {

struct layout_t {
    float2 x;
    float2 y;
    rect_f fill_extra;
    bool fill_x = false;
    bool fill_y = false;
    bool align_x = false;
    bool align_y = false;
};

}