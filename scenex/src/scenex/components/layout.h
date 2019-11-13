#pragma once

#include <scenex/config.h>

namespace scenex {

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