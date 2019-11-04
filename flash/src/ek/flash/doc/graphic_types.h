#pragma once

#include <vector>
#include <ek/math/vec.hpp>
#include <ek/math/mat3x2.hpp>

namespace ek::flash {

enum class scale_mode {
    none,
    normal,
    horizontal,
    vertical
};

enum class solid_style_type {
    hairline
};

enum class line_caps {
    none,
    round, // default
    square
};

enum class line_joints {
    miter,
    round, // default
    bevel
};

enum class fill_type {
    unknown = 0,
    solid = 1,
    linear = 2,
    radial = 3
};

enum class spread_method {
    extend = 0,
    reflect = 1,
    repeat = 2
};

struct solid_stroke {
    float4 fill;
    scale_mode scaleMode = scale_mode::none;
    solid_style_type solidStyle = solid_style_type::hairline;
    float weight = 1.0f;
    line_caps caps = line_caps::round;
    line_joints joints = line_joints::round;
    float miterLimit = 0.0f;
    bool pixelHinting = false;
};

struct gradient_entry {
    float4 color;
    float ratio = 0.0f;

    gradient_entry() = default;

    explicit gradient_entry(const float4& color, float ratio = 0.0f)
            : color{color},
              ratio{ratio} {
    }
};

struct fill_style {
    int index = 0;
    fill_type type;
    spread_method spreadMethod;
    std::vector<gradient_entry> entries;
    matrix_2d matrix;
};

struct stroke_style {
    int index = 0;
    solid_stroke solid;
    bool is_solid = false;
};

}


