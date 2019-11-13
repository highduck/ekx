#pragma once

#include <ek/math/vec_fwd.hpp>

namespace ek::flash {

enum class filter_kind_t {
    none = 0,
    drop_shadow = 1,
    glow = 2,
    blur = 3,
    bevel = 4,
    convolution = 5,
    adjust_color = 6
};

struct filter_t {
    filter_kind_t type = filter_kind_t::none;

    float4 color;
    float2 blur;
    float distance = 0.0f;
    float angle = 0.0f; // degrees
    uint8_t quality = 1; // TODO: check
    float strength = 100.0f;
    bool inner = false;
    bool knockout = false;
    bool hideObject = false;
};

}